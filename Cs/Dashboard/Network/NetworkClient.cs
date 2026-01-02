using System.Net.Sockets;

namespace Dashboard.Network;

public class NetworkClient
{
    private TcpClient? _tcpClient;
    private NetworkStream? _stream;
    private CancellationTokenSource? _cts;

    public event Action<GamePacket>? OnPacketReceived;
    public event Action<string>? OnLog;

    public bool IsConnected => _tcpClient != null && _tcpClient.Connected;

    public async Task ConnectAsync(string ip, int port)
    {
        try
        {
            if (_tcpClient != null) Disconnect();

            _tcpClient = new TcpClient();
            await _tcpClient.ConnectAsync(ip, port);
            _stream = _tcpClient.GetStream();
            _cts = new CancellationTokenSource();

            OnLog?.Invoke($"✅ Connesso a {ip}:{port}");

            // Avvia il loop
            // Nota: _cts non può essere null qui, ma per sicurezza usiamo il ? o ! se necessario
            if (_cts != null)
            {
                _ = Task.Run(() => ReceiveLoop(_cts.Token));
            }
        }
        catch (Exception ex)
        {
            OnLog?.Invoke($"❌ Errore connessione: {ex.Message}");
        }
    }

    private async Task ReceiveLoop(CancellationToken token)
    {
        // 1. LOCAL COPY: Catturiamo lo stream in una variabile locale sicura
        var localStream = _stream;

        // Se lo stream è null o non siamo connessi, usciamo subito
        if (localStream == null || !IsConnected) return;

        try
        {
            byte[] headerBuffer = new byte[8];

            while (!token.IsCancellationRequested && IsConnected)
            {
                // Passiamo 'localStream' (che siamo sicuri non sia null) all'helper
                if (!await ReadExactAsync(localStream, headerBuffer, 8, token)) break;

                uint typeVal = BitConverter.ToUInt32(headerBuffer, 0);
                uint sizeVal = BitConverter.ToUInt32(headerBuffer, 4);

                int bodySize = (int)sizeVal - 8;
                if (bodySize < 0) bodySize = 0;

                byte[] bodyBuffer = new byte[bodySize];
                if (bodySize > 0)
                {
                    // Passiamo sempre 'localStream'
                    if (!await ReadExactAsync(localStream, bodyBuffer, bodySize, token)) break;
                }

                var packet = new GamePacket
                {
                    Type = (PacketType)typeVal,
                    Data = bodyBuffer
                };

                OnPacketReceived?.Invoke(packet);
            }
        }
        catch (OperationCanceledException) { }
        catch (Exception ex)
        {
            OnLog?.Invoke($"⚠️ Errore ricezione: {ex.Message}");
            Disconnect();
        }
    }

    // Modificato per accettare 'NetworkStream stream' come parametro
    private async Task<bool> ReadExactAsync(NetworkStream stream, byte[] buffer, int count, CancellationToken token)
    {
        int totalRead = 0;
        while (totalRead < count)
        {
            // Qui usiamo l'argomento 'stream', che non è nullable, quindi il warning sparisce
            int read = await stream.ReadAsync(buffer, totalRead, count - totalRead, token);
            
            if (read == 0) return false;
            totalRead += read;
        }
        return true;
    }

    public void Disconnect()
    {
        _cts?.Cancel();
        _tcpClient?.Close();
        
        // Impostiamo a null i campi globali, ma il loop in esecuzione
        // finirà il suo lavoro graziosamente o genererà eccezione gestita
        _tcpClient = null;
        _stream = null; 
        
        OnLog?.Invoke("🔌 Disconnesso");
    }
    
    // Invia un pacchetto al server
    public async Task<bool> SendAsync(byte[] data)
    {
        if (_stream == null || !IsConnected)
        {
            OnLog?.Invoke("❌ Impossibile inviare: non connesso");
            return false;
        }
        
        try
        {
            await _stream.WriteAsync(data, 0, data.Length);
            return true;
        }
        catch (Exception ex)
        {
            OnLog?.Invoke($"❌ Errore invio: {ex.Message}");
            return false;
        }
    }
}