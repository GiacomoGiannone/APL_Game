using Dashboard.Network;
using System.Text;

namespace Dashboard;

public partial class MainPage : ContentPage
{
    private NetworkClient _client;
    
    // Dati giocatori con info complete
    private Dictionary<uint, PlayerInfo> _players = new();
    
    // Per compatibilità con RadarMap (usa solo posizioni)
    private Dictionary<uint, (float X, float Y)> _playerPositions = new();
    
    private uint? _selectedPlayerId = null;
    private IDispatcherTimer _uiTimer;
    private bool _playerListDirty = true; // Flag per aggiornare la lista solo quando necessario
    private DateTime _lastListUpdate = DateTime.MinValue;

    public MainPage()
    {
        InitializeComponent();
        
        // 1. Configura il Radar
        RadarView.Drawable = new RadarMap(_playerPositions);

        // 2. Configura Rete
        _client = new NetworkClient();
        _client.OnPacketReceived += HandlePacket;
        _client.OnLog += (msg) => MainThread.BeginInvokeOnMainThread(() => 
            AddLog(msg));

        // 3. Configura Timer UI (~30 FPS)
        _uiTimer = Dispatcher.CreateTimer();
        _uiTimer.Interval = TimeSpan.FromMilliseconds(33);
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
            AddLog("✅ Connesso come ADMIN");
        }
        else
        {
            _client.Disconnect();
            StatusLabel.Text = "OFFLINE";
            StatusLabel.TextColor = Colors.Red;
            lock (_players) 
            { 
                _players.Clear();
                _playerPositions.Clear();
            }
            _selectedPlayerId = null;
            UpdateAdminButtons();
            AddLog("🔌 Disconnesso");
        }
    }

    private void HandlePacket(GamePacket packet)
    {
        // Thread Rete - aggiorna dati
        if (packet.Type == PacketType.Move && packet.Data.Length >= 12)
        {
            uint id = BitConverter.ToUInt32(packet.Data, 0);
            float x = BitConverter.ToSingle(packet.Data, 4);
            float y = BitConverter.ToSingle(packet.Data, 8);

            lock (_players)
            {
                if (!_players.ContainsKey(id))
                {
                    _players[id] = new PlayerInfo { Id = id, Name = $"Player {id}" };
                    _playerListDirty = true;
                    MainThread.BeginInvokeOnMainThread(() => 
                        AddLog($"👤 Nuovo giocatore: Player {id}"));
                }
                
                _players[id].X = x;
                _players[id].Y = y;
                _players[id].LastUpdate = DateTime.Now;
                
                _playerPositions[id] = (x, y);
            }
        }
        else if (packet.Type == PacketType.PlayerDamage && packet.Data.Length >= 12)
        {
            uint id = BitConverter.ToUInt32(packet.Data, 0);
            float damage = BitConverter.ToSingle(packet.Data, 4);
            float health = BitConverter.ToSingle(packet.Data, 8);
            
            lock (_players)
            {
                if (_players.ContainsKey(id))
                {
                    _players[id].Health = health;
                    _playerListDirty = true;
                    
                    if (health <= 0)
                    {
                        MainThread.BeginInvokeOnMainThread(() => 
                            AddLog($"💀 Player {id} è MORTO!"));
                    }
                }
            }
        }
        else if (packet.Type == PacketType.PlayerDisconnected && packet.Data.Length >= 4)
        {
            uint id = BitConverter.ToUInt32(packet.Data, 0);
            
            lock (_players)
            {
                _players.Remove(id);
                _playerPositions.Remove(id);
                _playerListDirty = true;
            }
            
            MainThread.BeginInvokeOnMainThread(() => 
            {
                AddLog($"👋 Player {id} disconnesso");
                if (_selectedPlayerId == id)
                {
                    _selectedPlayerId = null;
                    UpdateAdminButtons();
                }
            });
        }
    }

    private void OnUpdateUiTick(object? sender, EventArgs e)
    {
        // 1. Ridisegna Radar
        RadarView.Invalidate();
        
        // 2. Aggiorna contatore
        PlayerCountLabel.Text = $"Giocatori: {_players.Count}";
        
        // 3. Aggiorna lista giocatori solo ogni 500ms (non ogni frame!)
        if (_playerListDirty || (DateTime.Now - _lastListUpdate).TotalMilliseconds > 500)
        {
            UpdatePlayerList();
            _playerListDirty = false;
            _lastListUpdate = DateTime.Now;
        }
        
        // 4. Rimuovi giocatori inattivi (timeout 10 sec)
        RemoveInactivePlayers();
    }
    
    private void UpdatePlayerList()
    {
        PlayerListPanel.Children.Clear();
        
        if (_players.Count == 0)
        {
            PlayerListPanel.Children.Add(new Label 
            { 
                Text = "Nessun giocatore connesso",
                TextColor = Colors.Gray,
                HorizontalOptions = LayoutOptions.Center,
                Margin = new Thickness(0, 20, 0, 0)
            });
            return;
        }
        
        lock (_players)
        {
            foreach (var player in _players.Values.OrderBy(p => p.Id))
            {
                var isSelected = _selectedPlayerId == player.Id;
                var healthPercent = player.MaxHealth > 0 ? player.Health / player.MaxHealth : 0;
                var healthColor = healthPercent > 0.5 ? Colors.Green : 
                                  healthPercent > 0.25 ? Colors.Orange : Colors.Red;
                
                var frame = new Frame
                {
                    BackgroundColor = isSelected ? Color.FromArgb("#2a4a2a") : Color.FromArgb("#252525"),
                    BorderColor = isSelected ? Colors.Lime : Colors.Gray,
                    CornerRadius = 5,
                    Padding = new Thickness(8, 5),
                    HasShadow = false
                };
                
                var grid = new Grid
                {
                    ColumnDefinitions = new ColumnDefinitionCollection
                    {
                        new ColumnDefinition(GridLength.Star),
                        new ColumnDefinition(GridLength.Auto)
                    }
                };
                
                var infoStack = new VerticalStackLayout { Spacing = 2 };
                
                infoStack.Children.Add(new Label
                {
                    Text = $"👤 Player {player.Id}",
                    TextColor = Colors.White,
                    FontAttributes = FontAttributes.Bold,
                    FontSize = 13
                });
                
                infoStack.Children.Add(new Label
                {
                    Text = $"📍 {player.X:F0}, {player.Y:F0}",
                    TextColor = Colors.Gray,
                    FontSize = 10
                });
                
                // Barra salute
                var healthBar = new ProgressBar
                {
                    Progress = healthPercent,
                    ProgressColor = healthColor,
                    HeightRequest = 6,
                    Margin = new Thickness(0, 3, 0, 0)
                };
                infoStack.Children.Add(healthBar);
                
                infoStack.Children.Add(new Label
                {
                    Text = $"❤️ {player.Health:F0}/{player.MaxHealth:F0}",
                    TextColor = healthColor,
                    FontSize = 10
                });
                
                grid.Children.Add(infoStack);
                Grid.SetColumn(infoStack, 0);
                
                // Status indicator
                var statusLabel = new Label
                {
                    Text = player.IsAlive ? "🟢" : "💀",
                    FontSize = 20,
                    VerticalOptions = LayoutOptions.Center
                };
                grid.Children.Add(statusLabel);
                Grid.SetColumn(statusLabel, 1);
                
                frame.Content = grid;
                
                // Click handler
                var tapGesture = new TapGestureRecognizer();
                var playerId = player.Id;
                tapGesture.Tapped += (s, e) => OnPlayerSelected(playerId);
                frame.GestureRecognizers.Add(tapGesture);
                
                PlayerListPanel.Children.Add(frame);
            }
        }
    }
    
    private void OnPlayerSelected(uint playerId)
    {
        _selectedPlayerId = playerId;
        SelectedPlayerLabel.Text = $"Selezionato: Player {playerId}";
        SelectedPlayerLabel.TextColor = Colors.Lime;
        UpdateAdminButtons();
        AddLog($"🎯 Selezionato Player {playerId}");
    }
    
    private void UpdateAdminButtons()
    {
        bool hasSelection = _selectedPlayerId.HasValue;
        KickButton.IsEnabled = hasSelection;
        BanButton.IsEnabled = hasSelection;
        
        if (!hasSelection)
        {
            SelectedPlayerLabel.Text = "Seleziona un giocatore";
            SelectedPlayerLabel.TextColor = Colors.Gray;
        }
    }
    
    private async void OnKickPlayer(object? sender, EventArgs e)
    {
        if (!_selectedPlayerId.HasValue || !_client.IsConnected) return;
        
        var playerId = _selectedPlayerId.Value;
        
        bool confirm = await DisplayAlert("Conferma KICK", 
            $"Vuoi kickare Player {playerId}?", "Sì", "No");
            
        if (confirm)
        {
            var packet = GamePacket.CreateKickPacket(playerId);
            await _client.SendAsync(packet);
            AddLog($"⚠️ KICK inviato a Player {playerId}");
        }
    }
    
    private async void OnBanPlayer(object? sender, EventArgs e)
    {
        if (!_selectedPlayerId.HasValue || !_client.IsConnected) return;
        
        var playerId = _selectedPlayerId.Value;
        
        bool confirm = await DisplayAlert("Conferma BAN", 
            $"Vuoi BANNARE Player {playerId}? Questa azione è permanente!", "Sì, Banna", "Annulla");
            
        if (confirm)
        {
            var packet = GamePacket.CreateBanPacket(playerId);
            await _client.SendAsync(packet);
            AddLog($"🚫 BAN inviato a Player {playerId}");
        }
    }
    
    private async void OnSpawnEnemy(object? sender, EventArgs e)
    {
        if (!_client.IsConnected)
        {
            await DisplayAlert("Errore", "Devi essere connesso per spawnare nemici", "OK");
            return;
        }
        
        // TODO: Implementare spawn nemico quando il server lo supporta
        AddLog("👹 Spawn nemico richiesto (non ancora implementato nel server)");
    }
    
    private void RemoveInactivePlayers()
    {
        var timeout = TimeSpan.FromSeconds(10);
        var now = DateTime.Now;
        
        lock (_players)
        {
            var inactive = _players.Where(p => now - p.Value.LastUpdate > timeout)
                                   .Select(p => p.Key)
                                   .ToList();
            
            foreach (var id in inactive)
            {
                _players.Remove(id);
                _playerPositions.Remove(id);
                
                if (_selectedPlayerId == id)
                {
                    _selectedPlayerId = null;
                    MainThread.BeginInvokeOnMainThread(UpdateAdminButtons);
                }
            }
        }
    }
    
    private void AddLog(string message)
    {
        var timestamp = DateTime.Now.ToString("HH:mm:ss");
        var newLog = $"[{timestamp}] {message}\n";
        
        // Mantieni solo le ultime 20 righe
        var lines = (LogLabel.Text ?? "").Split('\n').Take(19).ToArray();
        LogLabel.Text = newLog + string.Join("\n", lines);
    }
}