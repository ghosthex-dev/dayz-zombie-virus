modded class ZombieBase
{
	protected DayZPlayer m_ZV_Driver;
	protected AIAgent m_ZV_AIAgent;

	void ZV_SetDriver(DayZPlayer player)
	{
		m_ZV_Driver = player;
	}

	DayZPlayer ZV_GetDriver()
	{
		return m_ZV_Driver;
	}

	override bool ModCommandHandlerInside(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		m_ZV_AIAgent = GetAIAgent();

		if (!m_ZV_Driver)
		{
			if (m_ZV_AIAgent)
				m_ZV_AIAgent.SetKeepInIdle(false);
			return super.ModCommandHandlerInside(pDt, pCurrentCommandID, pCurrentCommandFinished);
		}

		if (m_ZV_AIAgent)
			m_ZV_AIAgent.SetKeepInIdle(true);

		PlayerBase driverPB = PlayerBase.Cast(m_ZV_Driver);
		if (!driverPB || !driverPB.IsAlive())
		{
			m_ZV_Driver = null;
			SetHealth("", "", 0);
			return false;
		}

		UAInterface input = driverPB.GetInputInterface();
		DayZInfectedInputController inputController = GetInputController();
		if (!input || !inputController)
			return false;

		// Steering: mouse horizontal aim delta and A/D keyboard strafe
		float mouseX = input.SyncedValue_ID(UAAimRight) - input.SyncedValue_ID(UAAimLeft);
		float keyX = input.SyncedValue_ID(UAMoveRight) - input.SyncedValue_ID(UAMoveLeft);
		float turnInput = mouseX;
		if (Math.AbsFloat(keyX) > 0.01)
			turnInput = keyX;

		float maxTurnSpeed = 120.0;
		turnInput = Math.Clamp(turnInput * maxTurnSpeed * pDt, -180, 180);
		if (input.SyncedValue_ID(UALookAround) > 0)
			turnInput = 0;

		float heading = GetOrientation()[0] + turnInput;
		inputController.OverrideTurnSpeed(true, Math.PI2 / pDt);
		inputController.OverrideHeading(true, heading * Math.DEG2RAD);

		// Movement forward / backward / sprint
		bool isMoveFwd = input.SyncedValue_ID(UAMoveForward) > 0;
		bool isMoveBack = input.SyncedValue_ID(UAMoveBack) > 0;
		bool isSprint = input.SyncedValue_ID(UATurbo) > 0;
		bool isJog = input.SyncedValue_ID(UAWalkRunTemp) > 0;

		float fSpeed = 0.0;
		int iAlert = 0;
		float fAlert = 0.0;

		if (isMoveFwd)
		{
			iAlert = 3;
			fAlert = 1.0;
			if (isSprint)
				fSpeed = 3.0; // sprint
			else if (isJog)
				fSpeed = 2.0; // run
			else
				fSpeed = 2.0; // responsive jog default
		}
		else if (isMoveBack)
		{
			iAlert = 3;
			fAlert = 1.0;
			fSpeed = 1.0; // walk back
		}

		inputController.OverrideMovementSpeed(true, fSpeed);
		inputController.OverrideAlertLevel(true, true, iAlert, fAlert);

		// Melee Attack (Left Click or Default Action)
		bool isAttack = (input.SyncedValue_ID(UAFire) > 0) || (input.SyncedValue_ID(UADefaultAction) > 0);
		if (isAttack && m_AttackCooldownTime <= 0 && pCurrentCommandID != DayZInfectedConstants.COMMANDID_ATTACK)
		{
			EntityAI target = ZV_FindTargetInFront(4.0, 60.0);
			if (target)
			{
				m_ActualTarget = target;
				float dist = vector.Distance(m_ActualTarget.GetPosition(), GetPosition());
				int pitch = GetAttackPitch(m_ActualTarget);

				m_ActualAttackType = GetDayZInfectedType().ChooseAttack(DayZInfectedAttackGroupType.FIGHT, dist, pitch);
				if (!m_ActualAttackType)
					m_ActualAttackType = GetDayZInfectedType().ChooseAttack(DayZInfectedAttackGroupType.CHASE, dist, pitch);

				if (m_ActualAttackType)
				{
					StartCommand_Attack(m_ActualTarget, m_ActualAttackType.m_Type, m_ActualAttackType.m_Subtype);
					m_AttackCooldownTime = m_ActualAttackType.m_Cooldown;
				}
			}
		}

		return true;
	}

	protected EntityAI ZV_FindTargetInFront(float maxDistance, float coneAngle)
	{
		vector origin = GetPosition();
		vector dir = GetDirection();

		ref array<typename> targetable = new array<typename>;
		ref array<Object> results = new array<Object>;

		targetable.Insert(PlayerBase);
		targetable.Insert(AnimalBase);

		Object target = DayZPlayerUtils.GetMeleeTarget(origin, dir, coneAngle, maxDistance, -1.0, 2.0, this, targetable, results);
		return EntityAI.Cast(target);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		if (m_ZV_Driver && damageResult)
		{
			float hpDmg = damageResult.GetHighestDamage("Health");
			if (hpDmg > 0)
				m_ZV_Driver.AddHealth("", "Health", -hpDmg);
		}
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		if (m_ZV_Driver)
		{
			PlayerBase driverPB = PlayerBase.Cast(m_ZV_Driver);
			m_ZV_Driver = null;
			if (driverPB && driverPB.IsAlive())
			{
				Print("[ZV] Puppet zombie of " + driverPB.ZV_Name() + " was slain; killing host");
				driverPB.SetHealth("", "", 0);
			}
		}
	}
}
