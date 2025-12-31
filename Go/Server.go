package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"sync"
	"time"
)

// Configurazione
const (
	PORT               = ":8080"
	LAN_DISCOVERY_PORT = 8888
	SERVER_NAME        = "APL Game Server"
)

// Tipi di pacchetti (deve corrispondere a C++)
const (
	PACKET_LOGIN               = 1
	PACKET_MOVE                = 2
	PACKET_PLAYER_DISCONNECTED = 3
	PACKET_ENEMY_SPAWN         = 4
	PACKET_ENEMY_UPDATE        = 5
	PACKET_ENEMY_DAMAGE        = 6
	PACKET_ENEMY_DEATH         = 7
	PACKET_PLAYER_ATTACK       = 8
	PACKET_HOST_ANNOUNCE       = 9
	PACKET_PLAYER_DAMAGE       = 10
)

// Struttura Client: rappresenta un giocatore connesso
type Client struct {
	conn net.Conn
	id   uint32
}

// Stato globale del server
var (
	clients   = make(map[uint32]*Client)     // Mappa di tutti i client connessi
	clientsMu sync.Mutex                     // Mutex per evitare crash quando due goroutine scrivono sulla mappa
	nextID    uint32                     = 1 // Contatore per assegnare ID univoci
)

// HEADER: Deve essere identico alla struct C++ PacketHeader
// C++: uint32 type, uint32 packetSize
type PacketHeader struct {
	Type       uint32
	PacketSize uint32
}

func main() {
	// 0. Avvia il broadcast LAN per la scoperta automatica
	go startLANDiscoveryBroadcast()

	// 1. Iniziamo ad ascoltare sulla porta TCP
	listener, err := net.Listen("tcp", PORT)
	if err != nil {
		fmt.Printf("Errore avvio server: %v\n", err)
		return
	}
	fmt.Printf("🚀 Server Go avviato su porta %s\n", PORT)
	fmt.Printf("📡 LAN Discovery attivo sulla porta UDP %d\n", LAN_DISCOVERY_PORT)

	// 2. Loop infinito: accetta nuove connessioni
	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Printf("Errore connessione: %v\n", err)
			continue
		}

		// 3. Per ogni client, avvia una "Goroutine" (thread leggero) separata
		go handleClient(conn)
	}
}

// Broadcast LAN per permettere ai client di trovare il server automaticamente
func startLANDiscoveryBroadcast() {
	// Prepara il pacchetto di annuncio
	// Formato: 4 byte magic ('APLG') + 2 byte porta + 32 byte nome server
	announcement := make([]byte, 38)
	copy(announcement[0:4], []byte("APLG"))                // Magic number
	binary.LittleEndian.PutUint16(announcement[4:6], 8080) // Porta del server TCP
	copy(announcement[6:38], []byte(SERVER_NAME))          // Nome server (32 byte max)

	// Ottieni tutti gli indirizzi broadcast delle interfacce di rete
	broadcastAddrs := getBroadcastAddresses()
	if len(broadcastAddrs) == 0 {
		fmt.Println("⚠️ Nessun indirizzo broadcast trovato, uso 255.255.255.255")
		broadcastAddrs = append(broadcastAddrs, "255.255.255.255")
	}

	fmt.Printf("📡 Broadcast LAN avviato su: %v\n", broadcastAddrs)

	// Loop infinito: manda broadcast ogni 2 secondi
	for {
		for _, addr := range broadcastAddrs {
			conn, err := net.DialUDP("udp4", nil, &net.UDPAddr{
				IP:   net.ParseIP(addr),
				Port: LAN_DISCOVERY_PORT,
			})
			if err != nil {
				continue
			}
			conn.Write(announcement)
			conn.Close()
		}
		time.Sleep(2 * time.Second)
	}
}

// Ottieni gli indirizzi broadcast di tutte le interfacce di rete
func getBroadcastAddresses() []string {
	var broadcasts []string

	interfaces, err := net.Interfaces()
	if err != nil {
		return broadcasts
	}

	for _, iface := range interfaces {
		// Salta interfacce down o loopback
		if iface.Flags&net.FlagUp == 0 || iface.Flags&net.FlagLoopback != 0 {
			continue
		}

		addrs, err := iface.Addrs()
		if err != nil {
			continue
		}

		for _, addr := range addrs {
			ipNet, ok := addr.(*net.IPNet)
			if !ok {
				continue
			}

			ip := ipNet.IP.To4()
			if ip == nil {
				continue // Salta IPv6
			}

			// Calcola l'indirizzo broadcast: IP | ~Mask
			mask := ipNet.Mask
			broadcast := make(net.IP, 4)
			for i := 0; i < 4; i++ {
				broadcast[i] = ip[i] | ^mask[i]
			}

			broadcastStr := broadcast.String()
			// Evita duplicati
			found := false
			for _, b := range broadcasts {
				if b == broadcastStr {
					found = true
					break
				}
			}
			if !found {
				broadcasts = append(broadcasts, broadcastStr)
				fmt.Printf("   - Interfaccia %s: broadcast %s\n", iface.Name, broadcastStr)
			}
		}
	}

	return broadcasts
}

func handleClient(conn net.Conn) {
	// Assegna un ID al nuovo client
	clientsMu.Lock() //Proteggiamo la mappa clients prima di scriverci dentro
	id := nextID
	nextID++
	client := &Client{conn: conn, id: id}
	clients[id] = client
	clientsMu.Unlock()

	fmt.Printf("➕ Nuovo Giocatore Connesso: ID %d (%s)\n", id, conn.RemoteAddr())

	// Invia al client il suo ID assegnato dal server
	// Pacchetto: Header (8 byte) + playerId (4 byte)
	welcomePacket := make([]byte, 12)
	binary.LittleEndian.PutUint32(welcomePacket[0:4], PACKET_LOGIN) // type
	binary.LittleEndian.PutUint32(welcomePacket[4:8], 12)           // size
	binary.LittleEndian.PutUint32(welcomePacket[8:12], id)          // playerId assegnato
	conn.Write(welcomePacket)
	fmt.Printf("   Inviato ID %d al client\n", id)

	// Assicurati di rimuovere il client quando la funzione finisce (disconnessione)
	defer func() {
		clientsMu.Lock()
		delete(clients, id)
		clientsMu.Unlock()
		conn.Close()
		fmt.Printf("➖ Giocatore Disconnesso: ID %d\n", id)
	}()

	// 4. Loop di lettura messaggi dal client
	for {
		// A. Leggi l'Header (8 Byte: 4 per Type + 4 per Size)
		// Usiamo LittleEndian perché i PC standard (x86/64) usano questo formato
		var header PacketHeader
		err := binary.Read(conn, binary.LittleEndian, &header)
		if err != nil {
			if err != io.EOF {
				fmt.Printf("Errore lettura header ID %d: %v\n", id, err)
			}
			return // Esci dal loop -> disconnessione
		}

		// B. Controllo di sicurezza sulla dimensione
		// (PacketSize include l'header stesso, quindi deve essere almeno 8)
		if header.PacketSize < 8 || header.PacketSize > 1024 {
			fmt.Printf("Pacchetto anomalo da ID %d: size %d\n", id, header.PacketSize)
			return
		}

		// C. Leggi il Corpo del pacchetto (Size - 8 bytes di header già letti)
		bodySize := header.PacketSize - 8
		body := make([]byte, bodySize)
		_, err = io.ReadFull(conn, body)
		if err != nil {
			fmt.Printf("Errore lettura body ID %d: %v\n", id, err)
			return
		}

		// D. Logica server: Qui potremmo modificare il pacchetto
		// Il server forza l'ID del pacchetto per sicurezza (prevenendo impersonificazioni)
		// (Il campo playerId è il primo campo (uint32) dopo l'header nel tuo MovePacket)
		if header.Type == PACKET_MOVE {
			// Sovrascriviamo i primi 4 byte del body con il vero ID del client
			binary.LittleEndian.PutUint32(body[0:4], id)
		}

		// Forza l'ID anche per PLAYER_ATTACK (il playerId è il primo campo del body)
		if header.Type == PACKET_PLAYER_ATTACK {
			binary.LittleEndian.PutUint32(body[0:4], id)
			fmt.Printf("⚔️ PLAYER_ATTACK da ID %d inoltrato\n", id)
		}

		// NON sovrascrivere l'ID per PLAYER_DAMAGE - l'ID è del player che subisce danno, non del mittente
		if header.Type == PACKET_PLAYER_DAMAGE {
			targetId := binary.LittleEndian.Uint32(body[0:4])
			fmt.Printf("💔 PLAYER_DAMAGE per player %d (inviato da %d)\n", targetId, id)
		}

		// E. INOLTRO (Broadcasting)
		// Ricostruiamo il pacchetto completo (Header + Body) per mandarlo agli altri
		fullPacket := make([]byte, header.PacketSize)

		// Scrivi header nel buffer
		binary.LittleEndian.PutUint32(fullPacket[0:4], header.Type)
		binary.LittleEndian.PutUint32(fullPacket[4:8], header.PacketSize)
		// Copia il body
		copy(fullPacket[8:], body)

		// PLAYER_DAMAGE va inviato a TUTTI (incluso il mittente) così l'host aggiorna il player remoto
		if header.Type == PACKET_PLAYER_DAMAGE {
			broadcastToAll(fullPacket)
		} else {
			broadcast(fullPacket, id)
		}
	}
}

// Invia il pacchetto a TUTTI tranne al mittente (senderID)
func broadcast(data []byte, senderID uint32) {
	clientsMu.Lock()
	defer clientsMu.Unlock()

	for id, client := range clients {
		if id != senderID {
			// Scrittura non bloccante (in un server reale si userebbero canali)
			_, err := client.conn.Write(data)
			if err != nil {
				fmt.Printf("Errore invio a ID %d\n", id)
			}
		}
	}
}

// Invia il pacchetto a TUTTI i client (incluso il mittente)
func broadcastToAll(data []byte) {
	clientsMu.Lock()
	defer clientsMu.Unlock()

	for id, client := range clients {
		_, err := client.conn.Write(data)
		if err != nil {
			fmt.Printf("Errore invio a ID %d\n", id)
		}
	}
}
