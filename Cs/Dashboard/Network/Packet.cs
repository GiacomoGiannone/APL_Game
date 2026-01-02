using System.IO;

namespace Dashboard.Network;

// Enum deve corrispondere a quello C++ (NetMessages.h)
public enum PacketType : uint
{
    None = 0,
    Login = 1,
    Move = 2,
    PlayerDisconnected = 3,
    EnemySpawn = 4,
    EnemyUpdate = 5,
    EnemyDamage = 6,
    EnemyDeath = 7,
    PlayerAttack = 8,
    HostAnnounce = 9,
    PlayerDamage = 10,
    
    // Comandi Admin (100+)
    AdminKick = 100,      // Kicka un giocatore
    AdminBan = 101,       // Banna un giocatore
    AdminMessage = 102,   // Messaggio broadcast dall'admin
    AdminSpawnEnemy = 103 // Spawna un nemico
}

// Info su un giocatore per la dashboard
public class PlayerInfo
{
    public uint Id { get; set; }
    public string Name { get; set; } = "";
    public float X { get; set; }
    public float Y { get; set; }
    public float Health { get; set; } = 100;
    public float MaxHealth { get; set; } = 100;
    public bool IsAlive => Health > 0;
    public DateTime LastUpdate { get; set; } = DateTime.Now;
}

public class GamePacket
{
    public PacketType Type { get; set; }
    public byte[] Data { get; set; } = Array.Empty<byte>();

    // Helper per creare pacchetti da inviare (se la dashboard dovrà dare comandi)
    public static byte[] Serialize(PacketType type, byte[] body)
    {
        uint bodySize = (uint)(body?.Length ?? 0);
        uint totalSize = 4 + 4 + bodySize; // 4 Type + 4 Size + Body

        using var ms = new MemoryStream();
        using var writer = new BinaryWriter(ms);
        
        writer.Write((uint)type);
        writer.Write(totalSize);
        if (body != null) writer.Write(body);
        
        return ms.ToArray();
    }
    
    // Helper per creare pacchetto Kick
    public static byte[] CreateKickPacket(uint playerId)
    {
        using var ms = new MemoryStream();
        using var writer = new BinaryWriter(ms);
        writer.Write(playerId);
        return Serialize(PacketType.AdminKick, ms.ToArray());
    }
    
    // Helper per creare pacchetto Ban
    public static byte[] CreateBanPacket(uint playerId)
    {
        using var ms = new MemoryStream();
        using var writer = new BinaryWriter(ms);
        writer.Write(playerId);
        return Serialize(PacketType.AdminBan, ms.ToArray());
    }
}