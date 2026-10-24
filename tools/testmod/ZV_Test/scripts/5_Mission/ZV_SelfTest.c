// Server self-test: spawns AI survivors and drives the infection by calling ZV_Tick directly
// (AI characters get no scheduled tick). Results go to the server RPT as "[ZV][TEST] ..." lines.
class ZV_SelfTest
{
	protected int m_Pass;
	protected int m_Fail;
	protected vector m_Base = "4500 0 2350";

	protected PlayerBase m_Turner;		// turns at the end of the incubation
	protected PlayerBase m_LateDeath;	// dies of other causes at stage 2 -> rises
	protected PlayerBase m_EarlyDeath;	// dies of other causes at stage 0 -> stays dead
	protected PlayerBase m_Bitten;		// left next to a real zombie
	protected vector m_TurnerPos;
	protected vector m_LatePos;
	protected vector m_EarlyPos;
	protected int m_ZombiesBefore1;
	protected int m_ZombiesBefore2;
	protected int m_ZombiesBefore3;

	void Check(bool ok, string what)
	{
		if (ok)
		{
			m_Pass++;
			Print("[ZV][TEST] PASS " + what);
		}
		else
		{
			m_Fail++;
			Print("[ZV][TEST] FAIL " + what);
		}
	}

	protected PlayerBase Npc(int slot)
	{
		vector pos = m_Base + Vector(slot * 10, 0, 0);
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]);
		return PlayerBase.Cast(g_Game.CreateObjectEx("SurvivorM_Mirek", pos, ECE_PLACE_ON_SURFACE));
	}

	protected void TickFor(PlayerBase p, float seconds)
	{
		while (seconds > 0 && p.IsAlive())
		{
			p.ZV_Tick(1);
			seconds = seconds - 1;
		}
	}

	protected int ZombiesNear(vector pos)
	{
		array<Object> objs = new array<Object>();
		g_Game.GetObjectsAtPosition(pos, 4, objs, null);
		int n = 0;
		foreach (Object o : objs)
		{
			if (ZombieBase.Cast(o))
				n++;
		}
		return n;
	}

	void Start()
	{
		ZV_TestState.s_On = true;
		ZV_Settings s = ZV_Virus.Get().GetSettings();
		Check(FileExist(ZV_Virus.SETTINGS_FILE), "settings.json written");
		Check(s.MaleZombies.Count() > 0, "male zombie list loaded");
		Check(s.FemaleZombies.Count() > 0, "female zombie list loaded");
		foreach (string zcls : s.MaleZombies)
			Check(g_Game.ConfigIsExisting("CfgVehicles " + zcls), "zombie class exists " + zcls);
		foreach (string fcls : s.FemaleZombies)
			Check(g_Game.ConfigIsExisting("CfgVehicles " + fcls), "zombie class exists " + fcls);

		// fast settings for the test
		s.IncubationMinutes = 1;
		s.TurnDelaySeconds = 2;
		s.InfectionChance = 1;
		s.InfectionChanceIfWounded = 1;

		// item config + script class
		vector itemPos = m_Base + Vector(0, 0, -5);
		itemPos[1] = g_Game.SurfaceY(itemPos[0], itemPos[2]);
		ZV_Antiviral cure = ZV_Antiviral.Cast(g_Game.CreateObjectEx("ZV_Antiviral", itemPos, ECE_PLACE_ON_SURFACE));
		Check(cure != null, "ZV_Antiviral spawns with its script class");
		TStringArray tex = new TStringArray();
		g_Game.ConfigGetTextArray("CfgVehicles ZV_Antiviral hiddenSelectionsTextures", tex);
		string texPath;
		if (tex.Count() > 0)
			texPath = tex[0];
		Check(texPath == "ZombieVirus\\data\\zv_antiviral_co.paa", "antiviral texture configured (" + texPath + ")");

		// chance 0 never infects, chance 1 always does
		PlayerBase immune = Npc(0);
		Check(immune != null, "AI survivor spawns");
		if (!immune || !cure)
		{
			Finish();
			return;
		}
		s.InfectionChance = 0;
		Check(!immune.ZV_OnZombieHit(false, false), "chance 0 does not infect");
		s.InfectionChance = 1;
		s.BlockedHitMultiplier = 0;
		Check(!immune.ZV_OnZombieHit(true, false), "blocked hit with multiplier 0 does not infect");
		s.BlockedHitMultiplier = 0.25;

		// progression through the stages
		PlayerBase a = Npc(1);
		Check(a.ZV_OnZombieHit(false, false), "chance 1 infects");
		Check(a.ZV_IsInfected(), "infected right after the bite");
		Check(a.ZV_GetStage() == 0, "stage 0 right after the bite");
		Check(a.ZV_GetHudStage() == 0, "HUD badge shows from the bite");
		Check(!a.ZV_OnZombieHit(false, false), "second bite does not re-infect");
		TickFor(a, 16);
		Check(a.ZV_GetStage() == 1, "stage 1 after 16 s of 60 (got " + a.ZV_GetStage() + ")");
		Check(a.ZV_GetHudStage() == 1, "HUD badge follows stage 1");
		Check(!a.HasDisease(), "vanilla sick icon not used");
		TickFor(a, 15);
		Check(a.ZV_GetStage() == 2, "stage 2 after 31 s (got " + a.ZV_GetStage() + ")");
		TickFor(a, 18);
		Check(a.ZV_GetStage() == 3, "stage 3 after 49 s (got " + a.ZV_GetStage() + ")");
		float hpBefore = a.GetHealth("", "Health");
		TickFor(a, 5);
		Check(a.GetHealth("", "Health") < hpBefore, "stage 3 drains health");

		// cure
		PlayerBase b = Npc(2);
		b.ZV_OnZombieHit(false, true);
		TickFor(b, 35);
		Check(b.ZV_GetStage() == 2, "second survivor at stage 2");
		cure.OnApply(b);
		Check(!b.ZV_IsInfected(), "antiviral cures");
		Check(b.ZV_GetStage() == -1, "no stage after the cure");
		Check(b.ZV_GetHudStage() == -1, "HUD badge cleared by the cure");
		Check(b.ZV_GetImmunity() > 1700, "antiviral gives immunity (" + b.ZV_GetImmunity() + " s)");
		Check(!b.ZV_OnZombieHit(false, true), "immune survivor is not infected");
		cure.Delete();

		// persistence
		PlayerBase e = Npc(3);
		e.ZV_OnZombieHit(false, false);
		TickFor(e, 36);
		ZV_Virus.Get().Store(e);
		ZV_Virus.Get().Save();
		ZV_SaveFile saved = new ZV_SaveFile();
		string err;
		bool loaded = JsonFileLoader<ZV_SaveFile>.LoadFile(ZV_Virus.PLAYERS_FILE, saved, err);
		Check(loaded, "players.json readable " + err);
		ZV_Record found;
		if (saved.Players)
		{
			foreach (ZV_Record rec : saved.Players)
			{
				if (rec.Id == e.ZV_Key())
					found = rec;
			}
		}
		Check(found != null, "record saved to players.json");
		if (found)
		{
			Check(found.Infected, "saved record is infected");
			Check(Math.AbsFloat(found.Progress - 0.6) < 0.02, "saved progress " + found.Progress);
		}
		e.ZV_SetState(false, 0, 0);
		Check(!e.ZV_IsInfected(), "state cleared before restore");
		ZV_Virus.Get().Restore(e);
		Check(e.ZV_IsInfected(), "infection restored from the record");
		Check(e.ZV_GetStage() == 2, "restored stage " + e.ZV_GetStage());
		string eKey = e.ZV_Key();
		e.SetHealth("", "", 0);
		Check(!ZV_Virus.Get().HasRecord(eKey), "record forgotten on death");

		// turning, and rising after death from other causes
		m_Turner = a;
		m_TurnerPos = a.GetPosition();
		m_ZombiesBefore1 = ZombiesNear(m_TurnerPos);
		TickFor(a, 10);
		Check(a.ZV_IsTurning(), "turns at the end of the incubation");

		m_LateDeath = Npc(4);
		m_LateDeath.ZV_OnZombieHit(false, false);
		TickFor(m_LateDeath, 32);
		m_LatePos = m_LateDeath.GetPosition();
		m_ZombiesBefore2 = ZombiesNear(m_LatePos);
		m_LateDeath.SetHealth("", "", 0);

		m_EarlyDeath = Npc(5);
		m_EarlyDeath.ZV_OnZombieHit(false, false);
		TickFor(m_EarlyDeath, 5);
		m_EarlyPos = m_EarlyDeath.GetPosition();
		m_ZombiesBefore3 = ZombiesNear(m_EarlyPos);
		m_EarlyDeath.SetHealth("", "", 0);

		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AfterRise, 4000, false);
	}

	void AfterRise()
	{
		Check(!m_Turner || !m_Turner.IsAlive(), "turned survivor is dead");
		Check(ZombiesNear(m_TurnerPos) > m_ZombiesBefore1, "zombie rose where the survivor turned");
		Check(ZombiesNear(m_LatePos) > m_ZombiesBefore2, "zombie rose from a stage 2 death");
		Check(ZombiesNear(m_EarlyPos) == m_ZombiesBefore3, "no zombie from a stage 0 death");

		// a real zombie next to a fresh survivor: only reported, AI may ignore an unplayed character
		m_Bitten = Npc(7);
		vector zpos = m_Bitten.GetPosition() + Vector(1.5, 0, 0);
		g_Game.CreateObjectEx("ZmbM_HermitSkinny_Beige", zpos, ECE_PLACE_ON_SURFACE | ECE_INITAI | ECE_EQUIP_ATTACHMENTS);
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AfterAttack, 40000, false);
	}

	void AfterAttack()
	{
		if (m_Bitten)
			Print("[ZV][TEST] INFO real zombie attack: health " + m_Bitten.GetHealth("", "Health") + ", infected " + m_Bitten.ZV_IsInfected());
		Finish();
	}

	void Finish()
	{
		ZV_TestState.s_On = false;
		Print("[ZV][TEST] DONE pass=" + m_Pass + " fail=" + m_Fail);
	}
}

modded class MissionServer
{
	protected ref ZV_SelfTest m_ZVSelfTest;

	override void OnInit()
	{
		super.OnInit();
		m_ZVSelfTest = new ZV_SelfTest();
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(m_ZVSelfTest.Start, 5000, false);
	}
}
