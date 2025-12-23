using Dashboard.Network;
using System.Text;

namespace Dashboard;

public partial class MainPage : ContentPage
{
    private NetworkClient _client;
    
    // I dati "vivi" dei giocatori
    private Dictionary<uint, (float X, float Y)> _playerPositions = new();
    
    private IDispatcherTimer _uiTimer;

    public MainPage()
    {
        InitializeComponent();
        
        // 1. Configura il Radar
        // Passiamo il riferimento del dizionario così la mappa sa cosa disegnare
        RadarView.Drawable = new RadarMap(_playerPositions);

        // 2. Configura Rete
        _client = new NetworkClient();
        _client.OnPacketReceived += HandlePacket;
        _client.OnLog += (msg) => Console.WriteLine($"[NET] {msg}");

        // 3. Configura Timer (60 FPS circa per fluidità)
        _uiTimer = Dispatcher.CreateTimer();
        _uiTimer.Interval = TimeSpan.FromMilliseconds(33); // ~30 FPS
        _uiTimer.Tick += OnUpdateUiTick;
        _uiTimer.Start();
    }

    private async void OnConnect(object? sender, EventArgs e)
    {
        if (!_client.IsConnected)
        {
            await _client.ConnectAsync("127.0.0.1", 8080);
            StatusLabel.Text = "ONLINE";
            StatusLabel.TextColor = Colors.Green;
        }
        else
        {
            _client.Disconnect();
            StatusLabel.Text = "OFFLINE";
            StatusLabel.TextColor = Colors.Red;
            lock (_playerPositions) { _playerPositions.Clear(); }
        }
    }

    // Nota: Il metodo OnSendMove è stato eliminato come richiesto.

    private void HandlePacket(GamePacket packet)
    {
        // Ricezione dati ad alta velocità (Thread Rete)
        if (packet.Type == PacketType.Move && packet.Data.Length >= 12)
        {
            uint id = BitConverter.ToUInt32(packet.Data, 0);
            float x = BitConverter.ToSingle(packet.Data, 4);
            float y = BitConverter.ToSingle(packet.Data, 8);

            lock (_playerPositions)
            {
                _playerPositions[id] = (x, y);
            }
        }
        else if (packet.Type == PacketType.PlayerJoined)
        {
            // Loggare eventi rari va bene nel testo
            MainThread.BeginInvokeOnMainThread(() => 
                LogLabel.Text = $"[{DateTime.Now:HH:mm:ss}] ⚠️ Rilevato nuovo segnale!\n" + LogLabel.Text);
        }
    }

    // Loop di rendering (Thread UI)
    private void OnUpdateUiTick(object? sender, EventArgs e)
    {
        // 1. Ridisegna il Radar (chiama il metodo Draw di RadarMap)
        RadarView.Invalidate();

        // 2. Aggiorna il Log testuale (solo se ci sono dati)
        if (_playerPositions.Count > 0)
        {
            var sb = new StringBuilder();
            // Mostra solo le ultime info testuali per debug
            sb.AppendLine("--- DATI TELEMETRICI ---");
            lock (_playerPositions)
            {
                foreach (var player in _playerPositions.OrderBy(p => p.Key))
                {
                    sb.AppendLine($"TGT {player.Key}: POS {player.Value.X:000.0}, {player.Value.Y:000.0}");
                }
            }
            // Aggiorna senza cancellare lo storico degli eventi importanti
            // Nota: Se vuoi solo vedere i numeri che cambiano, usa sb.ToString()
            // Se vuoi mantenere lo storico dei "PlayerJoined", dovremmo usare un'altra logica,
            // ma per ora sovrascriviamo per pulizia come richiesto.
            LogLabel.Text = sb.ToString(); 
        }
    }
}