using System;
using System.IO;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace Dashboard.Network
{
    public class NetworkClient
    {
        // Campi nullable per gestire lo stato di connessione
        private TcpClient? _client;
        private NetworkStream? _stream;
        private CancellationTokenSource? _cts;

        // Eventi per la UI
        public event Action<uint, byte[]>? OnPacketReceived;
        public event Action<string>? OnLogMessage;

        public bool IsConnected => _client != null && _client.Connected;

        public async Task ConnectAsync(string ip, int port)
        {
            // Chiude eventuali connessioni precedenti
            Disconnect();

            try
            {
                _client = new TcpClient();
                await _client.ConnectAsync(ip, port);
                _stream = _client.GetStream();
                _cts = new CancellationTokenSource();

                OnLogMessage?.Invoke($"✅ Connesso a {ip}:{port}");

                // Avvia il loop passando i riferimenti locali per evitare CS8602
                // Il '!' dice al compilatore: "Fidati, qui stream e cts non sono null"
                _ = Task.Run(() => ReadLoop(_stream, _cts.Token));
            }
            catch (Exception ex)
            {
                OnLogMessage?.Invoke($"❌ Errore connessione: {ex.Message}");
                Disconnect();
            }
        }

        // Metodo privato che gestisce la lettura continua
        // Riceve 'stream' come parametro per thread-safety (risolve CS8602)
        private async Task ReadLoop(NetworkStream stream, CancellationToken token)
        {
            try
            {
                while (!token.IsCancellationRequested)
                {
                    // 1. Leggi Header (8 byte)
                    byte[] headerBytes = await ReadExactAsync(stream, 8, token);
                    
                    uint type = BitConverter.ToUInt32(headerBytes, 0);
                    uint size = BitConverter.ToUInt32(headerBytes, 4);

                    // Controllo sicurezza dimensione
                    if (size < 8) throw new IOException("Pacchetto non valido (size < 8)");

                    // 2. Leggi Body
                    int bodySize = (int)size - 8;
                    byte[] body = bodySize > 0 
                        ? await ReadExactAsync(stream, bodySize, token) 
                        : Array.Empty<byte>();

                    // 3. Notifica
                    OnPacketReceived?.Invoke(type, body);
                }
            }
            catch (OperationCanceledException)
            {
                // Disconnessione volontaria, tutto ok
            }
            catch (Exception ex)
            {
                OnLogMessage?.Invoke($"⚠️ Disconnesso: {ex.Message}");
                Disconnect();
            }
        }

        // Helper per leggere esattamente N byte (risolve CA1835 usando AsMemory)
        private async Task<byte[]> ReadExactAsync(NetworkStream stream, int size, CancellationToken token)
        {
            byte[] buffer = new byte[size];
            int read = 0;

            while (read < size)
            {
                // Qui usiamo 'stream' locale, non '_stream' globale
                int received = await stream.ReadAsync(buffer.AsMemory(read, size - read), token);
                
                if (received == 0) throw new IOException("Connessione chiusa dal server");
                
                read += received;
            }
            return buffer;
        }

        public async Task SendAsync(byte[] data)
        {
            // Copia locale per thread-safety
            var localStream = _stream;
            if (localStream == null || !IsConnected) return;

            try
            {
                await localStream.WriteAsync(data.AsMemory());
            }
            catch (Exception ex)
            {
                OnLogMessage?.Invoke($"Errore invio: {ex.Message}");
                Disconnect();
            }
        }

        public void Disconnect()
        {
            _cts?.Cancel();     // Ferma il loop di lettura
            _client?.Close();   // Chiude il socket
            
            _client = null;
            _stream = null;
            _cts = null;
        }
    }
}