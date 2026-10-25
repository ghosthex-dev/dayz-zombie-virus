// Z-strain infection on the player. Everything here runs on the server only; symptoms reach the client
// through the vanilla symptom manager, the messages through NotificationSystem and the stage through
// m_ZV_HudStage (a net sync variable read by the HUD badge in ZV_IngameHud.c).
modded class PlayerBase
{
	static const float ZV_SYMPTOM_INTERVAL = 10;
	static const string ZV_ICON = "ZombieVirus/data/zv_biohazard_ca.paa";

	protected bool m_ZV_Infected;
	protected float m_ZV_Progress;		// 0..1
	protected float m_ZV_Immunity;		// seconds left
	protected int m_ZV_Stage = -1;		// -1 = not infected, 0..3 while infected
	protected float m_ZV_SymptomTimer;
	protected bool m_ZV_Turning;
	protected int m_ZV_HudStage = -1;	// synced copy of m_ZV_Stage for the owner's HUD
	protected bool m_ZV_Feral;
	protected float m_ZV_FeralTimer;
	protected ZombieBase m_ZV_PuppetZombie;
	protected bool m_ZV_HiddenMesh;
	protected bool m_ZV_HiddenMeshClient;

	override void Init()
	{
		super.Init();
		RegisterNetSyncVariableInt("m_ZV_HudStage", -1, 4);
		RegisterNetSyncVariableBool("m_ZV_Feral");
		RegisterNetSyncVariableObject("m_ZV_PuppetZombie");
		RegisterNetSyncVariableBool("m_ZV_HiddenMesh");
	}

	// Client side: stage shown on the HUD badge, -1 = not infected.
	int ZV_GetHudStage()
	{
		return m_ZV_HudStage;
	}

	ZombieBase ZV_GetPuppetZombie()
	{
		return m_ZV_PuppetZombie;
	}

	override void OnDrowningStart()
	{
		if (m_ZV_Feral)
			return;
		super.OnDrowningStart();
	}

	override bool GetDrowningWaterLevelCheck()
	{
		if (m_ZV_Feral)
			return false;
		return super.GetDrowningWaterLevelCheck();
	}

	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);
		if (m_ZV_HiddenMesh)
			ZV_HideBodyParts(true);
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (m_ZV_HiddenMesh != m_ZV_HiddenMeshClient)
		{
			ZV_HideBodyParts(m_ZV_HiddenMesh);
		}

		if (m_ZV_Feral)
		{
			ZombieBase zmb = m_ZV_PuppetZombie;
			if (!zmb && GetParent() && GetParent().IsInherited(ZombieBase))
				zmb = ZombieBase.Cast(GetParent());
			if (!zmb)
			{
				array<Object> nearbyObjs = new array<Object>;
				GetGame().GetObjectsAtPosition3D(GetPosition(), 5.0, nearbyObjs, null);
				for (int noi = 0; noi < nearbyObjs.Count(); noi++)
				{
					if (nearbyObjs[noi].IsInherited(ZombieBase))
					{
						zmb = ZombieBase.Cast(nearbyObjs[noi]);
						break;
					}
				}
			}

			if (zmb && !GetParent())
			{
				vector tmLocal[4];
				Math3D.MatrixIdentity4(tmLocal);
				LinkToLocalSpaceOf(zmb, tmLocal);
				dBodyEnableGravity(this, false);
			}

			ZV_HideBodyParts(true);
		}
		else if (!m_ZV_Feral)
		{
			if (GetParent())
			{
				UnlinkFromLocalSpace();
				dBodyEnableGravity(this, true);
			}
			ZV_HideBodyParts(false);
		}
	}

	void ZV_HideBodyParts(bool state)
	{
		if (state)
			ClearFlags(EntityFlags.VISIBLE, true);
		else
			SetFlags(EntityFlags.VISIBLE, true);

		SetInvisible(state);
		if (state)
			SetFaceTexture("");

		int decaySec = GetHiddenSelectionIndex("decay_preload");
		if (decaySec >= 0)
		{
			if (state)
				SetObjectTexture(decaySec, "");
			else
				SetObjectTexture(decaySec, m_DecayedTexture);
		}

		int personalitySec = GetHiddenSelectionIndex("personality");
		if (personalitySec >= 0)
			SetObjectTexture(personalitySec, "");

		int headSlotId = InventorySlots.GetSlotIdFromString("Head");
		EntityAI headEnt = GetInventory().FindPlaceholderForSlot(headSlotId);
		if (headEnt)
		{
			if (state)
				headEnt.ClearFlags(EntityFlags.VISIBLE, true);
			else
				headEnt.SetFlags(EntityFlags.VISIBLE, true);
			headEnt.SetInvisible(state);
			int headPers = headEnt.GetHiddenSelectionIndex("personality");
			if (headPers >= 0)
				headEnt.SetObjectTexture(headPers, "");
			int headHair = headEnt.GetHiddenSelectionIndex("hair");
			if (headHair >= 0)
				headEnt.SetObjectTexture(headHair, "");
			int headBeard = headEnt.GetHiddenSelectionIndex("beard");
			if (headBeard >= 0)
				headEnt.SetObjectTexture(headBeard, "");
		}

		EntityAI bodypart;
		int slot_id;
		array<string> bodyparts = {"Gloves","Body","Legs","Feet","Head","Hips","pelvis","leftleg","rightleg","Back","Mask","Hands","HeadGear","Eyewear","Armband","Vest","Shoulder","Melee","Bow"};
		for (int i = 0; i < bodyparts.Count(); i++)
		{
			slot_id = InventorySlots.GetSlotIdFromString(bodyparts.Get(i));
			bodypart = GetInventory().FindPlaceholderForSlot(slot_id);
			if (bodypart)
			{
				if (state)
					bodypart.ClearFlags(EntityFlags.VISIBLE, true);
				else
					bodypart.SetFlags(EntityFlags.VISIBLE, true);
				bodypart.SetInvisible(state);
				int bpPers = bodypart.GetHiddenSelectionIndex("personality");
				if (bpPers >= 0)
					bodypart.SetObjectTexture(bpPers, "");
			}

			EntityAI gear = FindAttachmentBySlotName(bodyparts.Get(i));
			if (gear)
			{
				if (state)
					gear.ClearFlags(EntityFlags.VISIBLE, true);
				else
					gear.SetFlags(EntityFlags.VISIBLE, true);
				gear.SetInvisible(state);
			}

			EntityAI hands = GetItemInHands();
			if (hands)
			{
				if (state)
					hands.ClearFlags(EntityFlags.VISIBLE, true);
				else
					hands.SetFlags(EntityFlags.VISIBLE, true);
				hands.SetInvisible(state);
			}
		}
		SetInvisibleRecursive(state, this, null);
		SetInvisible(state);
		m_ZV_HiddenMeshClient = state;
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		int zvBleedingBefore = GetBleedingSourceCount();
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!g_Game.IsServer() || !IsAlive())
			return;

		ZV_Settings settings = ZV_Virus.Get().GetSettings();

		// 1. Direct hit from another feral player character
		PlayerBase feralAttacker = PlayerBase.Cast(source);
		if (feralAttacker && feralAttacker.ZV_IsFeral())
		{
			ZV_Infect();
			if (damageResult)
			{
				float extraDmg = damageResult.GetHighestDamage("Health") * (settings.PlayableZombieDamageMultiplier - 1.0);
				if (extraDmg > 0)
					AddHealth("", "Health", -extraDmg);
			}
			BleedingSourcesManagerServer bm = GetBleedingManagerServer();
			if (bm)
			{
				if (!bm.AttemptAddBleedingSource(component))
					bm.AttemptAddBleedingSourceBySelection("Pelvis");
			}
			return;
		}

		// 2. Hit from a zombie entity
		ZombieBase zmb = ZombieBase.Cast(source);
		if (zmb)
		{
			// Player-controlled puppet zombie attack: 100% virulent infection, heavy claw bleed & feed healing
			if (zmb.ZV_GetDriver())
			{
				PlayerBase driver = PlayerBase.Cast(zmb.ZV_GetDriver());
				ZV_Infect();
				if (damageResult)
				{
					float zDmg = damageResult.GetHighestDamage("Health") * (settings.PlayableZombieDamageMultiplier - 1.0);
					if (zDmg > 0)
						AddHealth("", "Health", -zDmg);
				}
				BleedingSourcesManagerServer zbm = GetBleedingManagerServer();
				if (zbm)
				{
					if (!zbm.AttemptAddBleedingSource(component))
						zbm.AttemptAddBleedingSourceBySelection("Pelvis");
				}
				if (driver && driver.IsAlive())
				{
					driver.AddHealth("", "Health", settings.PlayableZombieHealthGainOnFeed);
				}
				Print("[ZV] Survivor " + ZV_Name() + " infected by player-controlled zombie driven by " + driver.ZV_Name());
				return;
			}

			// a blocked light attack only plays an animation
			if (ammo == "Dummy_Light")
				return;
			if (damageResult && damageResult.GetHighestDamage("Health") <= 0 && damageResult.GetHighestDamage("Blood") <= 0)
				return;

			bool zvBlocked = false;
			if (GetMeleeFightLogic() && GetMeleeFightLogic().IsInBlock())
				zvBlocked = true;
			bool zvWounded = false;
			if (GetBleedingSourceCount() > zvBleedingBefore)
				zvWounded = true;
			ZV_OnZombieHit(zvBlocked, zvWounded);
		}
	}

	override void OnScheduledTick(float deltaTime)
	{
		super.OnScheduledTick(deltaTime);
		if (g_Game.IsServer() && IsPlayerSelected() && IsAlive())
			ZV_Tick(deltaTime);
	}

	override void OnConnect()
	{
		super.OnConnect();
		if (g_Game.IsServer())
			ZV_Virus.Get().Restore(this);
	}

	override void OnDisconnect()
	{
		if (g_Game.IsServer() && IsAlive())
		{
			ZV_Virus.Get().Store(this);
			ZV_Virus.Get().Save();
		}
		super.OnDisconnect();
	}

	override void EEKilled(Object killer)
	{
		if (g_Game.IsServer())
		{
			ZV_Settings s = ZV_Virus.Get().GetSettings();
			if (m_ZV_Feral)
			{
				Print("[ZV] Feral player " + ZV_Name() + " was slain");
				m_ZV_Feral = false;
				if (m_ZV_PuppetZombie)
				{
					ZombieBase zmb = m_ZV_PuppetZombie;
					m_ZV_PuppetZombie = null;
					zmb.ZV_SetDriver(null);
					if (zmb.IsAlive())
						zmb.SetHealth("", "", 0);
					SetPosition(zmb.GetPosition());
				}
				UnlinkFromLocalSpace();
				dBodyEnableGravity(this, true);
				m_ZV_HiddenMesh = false;
				ZV_HideBodyParts(false);
				if (GetInventory())
					GetInventory().UnlockInventory(LOCK_FROM_SCRIPT);
				if (GetIdentity())
					GetGame().SelectPlayer(GetIdentity(), this);
			}
			else if (!m_ZV_Turning && m_ZV_Infected && m_ZV_Stage >= s.RiseOnDeathMinStage)
			{
				m_ZV_Turning = true;
				vector zvOri = GetOrientation();
				ZV_Virus.Get().QueueRise(GetPosition(), zvOri[0], IsMale(), this);
				Print("[ZV] " + ZV_Name() + " died infected (stage " + m_ZV_Stage + ") and will rise");
			}
			ZV_ClearInfection();
			m_ZV_Immunity = 0;
			ZV_Virus.Get().Forget(ZV_Key());
		}
		super.EEKilled(killer);
	}

	// ---- API (also used by the antiviral and the self-test) ----

	string ZV_Key()
	{
		if (GetIdentity())
			return GetIdentity().GetId();
		return "";	// AI characters are never saved
	}

	bool ZV_IsInfected()
	{
		return m_ZV_Infected;
	}

	float ZV_GetProgress()
	{
		return m_ZV_Progress;
	}

	int ZV_GetStage()
	{
		return m_ZV_Stage;
	}

	float ZV_GetImmunity()
	{
		return m_ZV_Immunity;
	}

	bool ZV_IsTurning()
	{
		return m_ZV_Turning;
	}

	bool ZV_IsFeral()
	{
		return m_ZV_Feral;
	}

	float ZV_GetFeralTimer()
	{
		return m_ZV_FeralTimer;
	}

	// Rolls the infection chance for one zombie hit. Returns true when it infected the player.
	bool ZV_OnZombieHit(bool blocked, bool wounded)
	{
		if (!IsAlive() || m_ZV_Infected || m_ZV_Turning || m_ZV_Feral || m_ZV_Immunity > 0)
			return false;
		ZV_Settings s = ZV_Virus.Get().GetSettings();
		float chance = s.InfectionChance;
		if (wounded)
			chance = s.InfectionChanceIfWounded;
		if (blocked)
			chance = chance * s.BlockedHitMultiplier;
		if (Math.RandomFloat01() >= chance)
			return false;
		ZV_Infect();
		return true;
	}

	void ZV_Infect()
	{
		if (m_ZV_Infected || !IsAlive())
			return;
		m_ZV_Infected = true;
		m_ZV_Progress = 0;
		m_ZV_SymptomTimer = 0;
		ZV_UpdateStage(true);
		ZV_Virus.Get().Store(this);
		Print("[ZV] " + ZV_Name() + " infected");
	}

	// Antiviral injection: kills the virus at any stage and gives temporary immunity.
	void ZV_Cure()
	{
		if (!g_Game.IsServer() || m_ZV_Turning || m_ZV_Feral || !IsAlive())
			return;
		bool wasInfected = m_ZV_Infected;
		ZV_ClearInfection();
		m_ZV_Immunity = ZV_Virus.Get().GetSettings().CureImmunityMinutes * 60;
		ZV_Virus.Get().Store(this);
		ZV_Virus.Get().Save();
		if (wasInfected)
		{
			ZV_Notify("The fever breaks.", "The antiviral is working. You are immune for a while.");
			Print("[ZV] " + ZV_Name() + " cured");
		}
		else
		{
			ZV_Notify("Immunised.", "The antiviral will protect you for a while.");
		}
	}

	// Restores a saved state without messages.
	void ZV_SetState(bool infected, float progress, float immunity, bool feral = false, float feralTimer = 0)
	{
		ZV_ClearInfection();
		m_ZV_Immunity = Math.Max(immunity, 0);
		if (feral)
		{
			m_ZV_Feral = true;
			m_ZV_HudStage = 4;
			m_ZV_FeralTimer = feralTimer;
			SetSynchDirty();
			ZV_BecomeFeral();
			return;
		}
		if (!infected)
			return;
		m_ZV_Infected = true;
		m_ZV_Progress = Math.Clamp(progress, 0, 1);
		ZV_UpdateStage(false);
	}

	// One step of the infection. Called from the scheduled tick; public so the self-test can drive it.
	void ZV_Tick(float dt)
	{
		if (m_ZV_Immunity > 0)
		{
			m_ZV_Immunity = m_ZV_Immunity - dt;
			if (m_ZV_Immunity <= 0)
			{
				m_ZV_Immunity = 0;
				ZV_Virus.Get().Store(this);
			}
		}
		if (m_ZV_Feral)
		{
			ZV_FeralTick(dt);
			return;
		}

		if (!m_ZV_Infected || m_ZV_Turning || !IsAlive())
			return;

		ZV_Settings s = ZV_Virus.Get().GetSettings();
		float rate = 1;
		if (IsAntibioticsActive())
			rate = s.AntibioticsSpeedMultiplier;
		m_ZV_Progress = m_ZV_Progress + dt * rate / (s.IncubationMinutes * 60);
		if (m_ZV_Progress >= 1)
		{
			m_ZV_Progress = 1;
			ZV_Turn();
			return;
		}
		ZV_UpdateStage(true);

		if (m_ZV_Stage >= 3 && s.Stage3HealthDrainPerMinute > 0)
			AddHealth("", "Health", -s.Stage3HealthDrainPerMinute * dt / 60);

		m_ZV_SymptomTimer = m_ZV_SymptomTimer + dt;
		if (m_ZV_SymptomTimer >= ZV_SYMPTOM_INTERVAL)
		{
			m_ZV_SymptomTimer = 0;
			ZV_RollSymptoms();
		}
	}

	// ---- internals ----

	protected int ZV_StageFor(float progress)
	{
		ZV_Settings s = ZV_Virus.Get().GetSettings();
		if (progress >= s.Stage3At)
			return 3;
		if (progress >= s.Stage2At)
			return 2;
		if (progress >= s.Stage1At)
			return 1;
		return 0;
	}

	protected void ZV_UpdateStage(bool notify)
	{
		int stage = ZV_StageFor(m_ZV_Progress);
		if (stage == m_ZV_Stage)
			return;
		int oldStage = m_ZV_Stage;
		ZV_ApplyStageEffects(stage);
		m_ZV_Stage = stage;
		if (!notify || stage < oldStage)
			return;
		switch (stage)
		{
			case 0:
				ZV_Notify("The bite burns.", "Something got into the wound. Find an antiviral before it spreads.");
				break;
			case 1:
				ZV_Notify("You feel feverish.", "Chills and a dry cough. The infection is spreading.");
				break;
			case 2:
				ZV_Notify("Your veins burn.", "Nausea and pain. Without an antiviral you will not last long.");
				break;
			case 3:
				ZV_Notify("You are losing yourself.", "Your thoughts slip away. Find an antiviral now.");
				break;
		}
	}

	// Persistent effects of a stage; m_ZV_Stage still holds the previous stage here.
	protected void ZV_ApplyStageEffects(int stage)
	{
		if (m_ZV_HudStage != stage)
		{
			m_ZV_HudStage = stage;
			SetSynchDirty();
		}
	}

	protected void ZV_ClearInfection()
	{
		ZV_ApplyStageEffects(-1);
		m_ZV_Infected = false;
		m_ZV_Feral = false;
		m_ZV_FeralTimer = 0;
		m_ZV_Progress = 0;
		m_ZV_Stage = -1;
		m_ZV_SymptomTimer = 0;

		if (m_ZV_PuppetZombie)
		{
			ZombieBase zmb = m_ZV_PuppetZombie;
			m_ZV_PuppetZombie = null;
			zmb.ZV_SetDriver(null);
			if (zmb.IsAlive())
				zmb.SetHealth("", "", 0);
			SetPosition(zmb.GetPosition());
		}
		UnlinkFromLocalSpace();
		dBodyEnableGravity(this, true);
		m_ZV_HiddenMesh = false;
		ZV_HideBodyParts(false);
		if (GetInventory())
			GetInventory().UnlockInventory(LOCK_FROM_SCRIPT);
		if (GetIdentity())
			GetGame().SelectPlayer(GetIdentity(), this);

		SetSynchDirty();
	}

	protected void ZV_RollSymptoms()
	{
		SymptomManager sm = GetSymptomManager();
		if (!sm || IsUnconscious())
			return;
		float roll = Math.RandomFloat01();
		switch (m_ZV_Stage)
		{
			case 1:
				if (roll < 0.10)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_COUGH);
				else if (roll < 0.15)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_SNEEZE);
				break;
			case 2:
				if (roll < 0.15)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_COUGH);
				else if (roll < 0.19)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_VOMIT);
				else if (roll < 0.29)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_PAIN_LIGHT);
				break;
			case 3:
				if (roll < 0.15)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_COUGH);
				else if (roll < 0.21)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_VOMIT);
				else if (roll < 0.33)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_PAIN_HEAVY);
				else if (roll < 0.38)
					sm.QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_LAUGHTER);
				if (Math.RandomFloat01() < 0.3)
					DayZPlayerSyncJunctures.SendKuruRequest(this, 0.3);
				break;
		}
	}

	protected void ZV_Turn()
	{
		ZV_Settings s = ZV_Virus.Get().GetSettings();
		if (s.PlayableZombie)
		{
			ZV_BecomeFeral();
			return;
		}

		m_ZV_Turning = true;
		ZV_Notify("You have turned.", "The virus has taken you.");
		Print("[ZV] " + ZV_Name() + " turned");
		vector ori = GetOrientation();
		ZV_Virus.Get().QueueRise(GetPosition(), ori[0], IsMale(), this);
		SetHealth("", "", 0);
	}

	void ZV_BecomeFeral()
	{
		m_ZV_Feral = true;
		m_ZV_Turning = false;
		m_ZV_HudStage = 4;
		m_ZV_Progress = 1.0;
		m_ZV_FeralTimer = 0;

		EntityAI inHands = GetItemInHands();
		if (inHands)
		{
			vector m4[4];
			Math3D.MatrixIdentity4(m4);
			GameInventory.PrepareDropEntityPos(this, inHands, m4, false, -1);
			InventoryLocation groundLoc = new InventoryLocation();
			groundLoc.SetGround(inHands, m4);
			InventoryLocation srcLoc = new InventoryLocation();
			if (inHands.GetInventory().GetCurrentInventoryLocation(srcLoc))
				GameInventory.LocationSyncMoveEntity(srcLoc, groundLoc);
			if (GetItemInHands() && GetIdentity())
				ServerDropEntity(inHands);
		}

		if (GetInventory())
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);

		// Spawn the physical puppet zombie
		ZV_Settings s = ZV_Virus.Get().GetSettings();
		string zcls = "ZmbM_HermitSkinny_Beige";
		if (IsMale() && s.MaleZombies.Count() > 0)
			zcls = s.MaleZombies.GetRandomElement();
		else if (!IsMale() && s.FemaleZombies.Count() > 0)
			zcls = s.FemaleZombies.GetRandomElement();

		vector pPos = GetPosition();
		vector pDir = GetDirection();
		vector pOri = GetOrientation();

		// Spawn zombie 1.8m in front of player so physics does not shove it offset
		vector zSpawnPos = pPos + (pDir * 1.8);
		zSpawnPos[1] = g_Game.SurfaceY(zSpawnPos[0], zSpawnPos[2]);

		ZombieBase zombie = ZombieBase.Cast(g_Game.CreateObjectEx(zcls, zSpawnPos, ECE_PLACE_ON_SURFACE | ECE_INITAI));
		if (zombie)
		{
			zombie.SetOrientation(pOri);
			zombie.ZV_SetDriver(this);
			m_ZV_PuppetZombie = zombie;

			// Attach player directly to zombie root in local space
			vector tmLocal[4];
			Math3D.MatrixIdentity4(tmLocal);
			LinkToLocalSpaceOf(zombie, tmLocal);
			dBodyEnableGravity(this, false);


		}

		m_ZV_HiddenMesh = true;
		ZV_HideBodyParts(true);
		SetSynchDirty();

		DayZPlayerSyncJunctures.SendKuruRequest(this, 1.0);
		ZV_Notify("You have turned.", "Your humanity is gone. You are now an infected. Hunt the living.");
		Print("[ZV] " + ZV_Name() + " became a PLAYABLE FERAL ZOMBIE (Puppet: " + zcls + ")");
		ZV_Virus.Get().Store(this);
		ZV_Virus.Get().Save();
	}

	protected void ZV_FeralTick(float dt)
	{
		ZV_Settings s = ZV_Virus.Get().GetSettings();
		m_ZV_FeralTimer += dt;

		if (s.PlayableZombieHealthDrainPerMinute > 0)
			AddHealth("", "Health", -s.PlayableZombieHealthDrainPerMinute * dt / 60);

		m_ZV_SymptomTimer += dt;
		if (m_ZV_SymptomTimer >= 15)
		{
			m_ZV_SymptomTimer = 0;
			if (Math.RandomFloat01() < 0.4)
				DayZPlayerSyncJunctures.SendKuruRequest(this, 0.5);
		}

		if (s.PlayableZombieDurationMinutes > 0 && m_ZV_FeralTimer >= s.PlayableZombieDurationMinutes * 60)
		{
			Print("[ZV] Feral body of " + ZV_Name() + " expired");
			SetHealth("", "", 0);
		}
	}

	override bool CanReceiveItemIntoHands(EntityAI item_to_hands)
	{
		if (m_ZV_Feral)
		{
			if (item_to_hands && item_to_hands.IsMeat())
				return true;
			return false;
		}
		return super.CanReceiveItemIntoHands(item_to_hands);
	}

	override bool CanBeTargetedByAI(EntityAI ai)
	{
		if (m_ZV_Feral && ZombieBase.Cast(ai) && ZV_Virus.Get().GetSettings().PlayableZombieIgnoredByAI)
			return false;
		return super.CanBeTargetedByAI(ai);
	}

	override bool Consume(PlayerConsumeData data)
	{
		if (m_ZV_Feral && data && data.m_Source && data.m_Source.IsMeat())
		{
			ZV_Settings settings = ZV_Virus.Get().GetSettings();
			AddHealth("", "Health", settings.PlayableZombieHealthGainOnFeed);
			data.m_Agents = 0;
		}
		return super.Consume(data);
	}

	protected void ZV_Notify(string title, string text)
	{
		// without an identity the notification RPC would go to every client
		if (GetIdentity() && ZV_Virus.Get().GetSettings().NotifyPlayer)
			NotificationSystem.SendNotificationToPlayerExtended(this, 8, title, text, ZV_ICON);
	}

	string ZV_Name()
	{
		if (GetIdentity())
			return GetIdentity().GetName() + " (" + GetIdentity().GetId() + ")";
		return GetType() + " " + GetID();
	}
}
