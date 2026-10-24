// One saved player: infection state survives logouts and server restarts.
class ZV_Record
{
	string Id;
	bool Infected;
	float Progress;		// 0..1, 1 = turns
	float Immunity;		// seconds left
}

class ZV_SaveFile
{
	int Version = 1;
	ref array<ref ZV_Record> Players = new array<ref ZV_Record>;
}

// Server-side owner of the settings and the saved infections. The live state of an online player sits on
// PlayerBase (ZV_PlayerBase.c); this class copies it in on connect and out every 30 s and on logout.
// Kept in its own JSON file instead of the character save so adding or removing the mod never breaks characters.
class ZV_Virus
{
	static const string DIR = "$profile:ZombieVirus";
	static const string SETTINGS_FILE = "$profile:ZombieVirus/settings.json";
	static const string PLAYERS_FILE = "$profile:ZombieVirus/players.json";
	static const float SAVE_INTERVAL = 30;

	protected static ref ZV_Virus s_Instance;

	protected ref ZV_Settings m_Settings;
	protected ref map<string, ref ZV_Record> m_Records;
	protected float m_SaveTimer;
	protected bool m_Dirty;

	static ZV_Virus Get()
	{
		if (!s_Instance)
			s_Instance = new ZV_Virus();
		return s_Instance;
	}

	void ZV_Virus()
	{
		m_Settings = new ZV_Settings();
		m_Records = new map<string, ref ZV_Record>();
		if (g_Game.IsServer())
		{
			if (!FileExist(DIR))
				MakeDirectory(DIR);
			LoadSettings();
			LoadRecords();
		}
	}

	ZV_Settings GetSettings()
	{
		return m_Settings;
	}

	protected void LoadSettings()
	{
		string err;
		if (FileExist(SETTINGS_FILE))
		{
			if (!JsonFileLoader<ZV_Settings>.LoadFile(SETTINGS_FILE, m_Settings, err))
			{
				Print("[ZV] settings.json is broken, using defaults: " + err);
				m_Settings = new ZV_Settings();
				m_Settings.FillDefaultZombies();
			}
		}
		else
		{
			m_Settings.FillDefaultZombies();
			if (!JsonFileLoader<ZV_Settings>.SaveFile(SETTINGS_FILE, m_Settings, err))
				Print("[ZV] cannot write settings.json: " + err);
		}
		m_Settings.Validate();
		Print("[ZV] settings: chance " + m_Settings.InfectionChance + " / wounded " + m_Settings.InfectionChanceIfWounded + ", incubation " + m_Settings.IncubationMinutes + " min");
	}

	protected void LoadRecords()
	{
		if (!FileExist(PLAYERS_FILE))
			return;
		ZV_SaveFile data = new ZV_SaveFile();
		string err;
		if (!JsonFileLoader<ZV_SaveFile>.LoadFile(PLAYERS_FILE, data, err))
		{
			Print("[ZV] players.json is broken, starting empty: " + err);
			return;
		}
		if (!data.Players)
			return;
		foreach (ZV_Record rec : data.Players)
		{
			if (rec && rec.Id != "")
				m_Records.Set(rec.Id, rec);
		}
		Print("[ZV] loaded " + m_Records.Count() + " saved infections");
	}

	void Save()
	{
		ZV_SaveFile data = new ZV_SaveFile();
		foreach (string id, ZV_Record rec : m_Records)
		{
			data.Players.Insert(rec);
		}
		string err;
		if (!JsonFileLoader<ZV_SaveFile>.SaveFile(PLAYERS_FILE, data, err))
			Print("[ZV] cannot write players.json: " + err);
		m_Dirty = false;
	}

	// Copies the live state of every online player into the records and writes the file if anything changed.
	void SaveAll()
	{
		array<Man> players = new array<Man>();
		g_Game.GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.IsAlive())
				Store(pb);
		}
		if (m_Dirty)
			Save();
	}

	void OnUpdate(float timeslice)
	{
		m_SaveTimer += timeslice;
		if (m_SaveTimer < SAVE_INTERVAL)
			return;
		m_SaveTimer = 0;
		SaveAll();
	}

	// Live state -> record. Players with nothing to remember have no record.
	void Store(PlayerBase player)
	{
		string id = player.ZV_Key();
		if (id == "")
			return;
		if (!player.ZV_IsInfected() && player.ZV_GetImmunity() <= 0)
		{
			if (m_Records.Contains(id))
			{
				m_Records.Remove(id);
				m_Dirty = true;
			}
			return;
		}
		ZV_Record rec = m_Records.Get(id);
		if (!rec)
		{
			rec = new ZV_Record();
			rec.Id = id;
			m_Records.Set(id, rec);
		}
		rec.Infected = player.ZV_IsInfected();
		rec.Progress = player.ZV_GetProgress();
		rec.Immunity = player.ZV_GetImmunity();
		m_Dirty = true;
	}

	// Record -> live state, when the player connects.
	void Restore(PlayerBase player)
	{
		ZV_Record rec = m_Records.Get(player.ZV_Key());
		if (rec)
			player.ZV_SetState(rec.Infected, rec.Progress, rec.Immunity);
	}

	bool HasRecord(string id)
	{
		return m_Records.Contains(id);
	}

	// The character is gone (died, turned or was replaced by a new one).
	void Forget(string id)
	{
		if (id == "" || !m_Records.Contains(id))
			return;
		m_Records.Remove(id);
		Save();
	}

	void QueueRise(vector pos, float yaw, bool male, PlayerBase body)
	{
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Rise, m_Settings.TurnDelaySeconds * 1000, false, pos, yaw, male, body);
	}

	protected void Rise(vector pos, float yaw, bool male, PlayerBase body)
	{
		if (body)
			pos = body.GetPosition();
		string cls = PickZombie(male);
		Object zombie = g_Game.CreateObjectEx(cls, pos, ECE_PLACE_ON_SURFACE | ECE_INITAI | ECE_EQUIP_ATTACHMENTS);
		if (!zombie)
		{
			Print("[ZV] cannot spawn zombie " + cls);
			return;
		}
		zombie.SetOrientation(Vector(yaw, 0, 0));
		Print("[ZV] rose as " + cls + " at " + pos.ToString());
		if (body && m_Settings.DeleteBodyOnTurn)
			body.Delete();
	}

	string PickZombie(bool male)
	{
		array<string> list = m_Settings.FemaleZombies;
		if (male)
			list = m_Settings.MaleZombies;
		return list.GetRandomElement();
	}
}
