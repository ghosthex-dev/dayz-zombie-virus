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
		HumanInputController hic = driverPB.GetInputController();
		DayZInfectedInputController inputController = GetInputController();
		if (!inputController)
			return false;

		// Decrement attack cooldown
		if (m_AttackCooldownTime > 0)
			m_AttackCooldownTime -= pDt * 1.5;

		// Handle ongoing attack command: check hit frame and apply damage
		if (pCurrentCommandID == DayZInfectedConstants.COMMANDID_ATTACK)
		{
			DayZInfectedCommandAttack attackCmd = GetCommand_Attack();
			if (attackCmd && attackCmd.WasHit())
			{
				if (m_ActualTarget)
				{
					vector hitPosWS = m_ActualTarget.ModelToWorld(m_ActualTarget.GetDefaultHitPosition());
					string ammo = "MeleeZombie";
					if (m_ActualAttackType && m_ActualAttackType.m_AmmoType != "")
						ammo = m_ActualAttackType.m_AmmoType;
					DamageSystem.CloseCombatDamageName(this, m_ActualTarget, m_ActualTarget.GetHitComponentForAI(), ammo, hitPosWS);
				}
				else
				{
					// If swung into thin air, check if a target moved within strike reach (2.5m, 70 deg cone)
					EntityAI strikeTarget = ZV_FindTargetInFront(2.5, 70.0);
					if (strikeTarget)
					{
						vector strikePos = strikeTarget.ModelToWorld(strikeTarget.GetDefaultHitPosition());
						DamageSystem.CloseCombatDamageName(this, strikeTarget, strikeTarget.GetHitComponentForAI(), "MeleeZombie", strikePos);
					}
				}
			}
			return true;
		}

		// Detect attack input (Left Mouse Click / Default Action)
		bool isAttack = false;
		if (input)
		{
			if (input.SyncedValue_ID(UAFire) > 0 || input.SyncedPress_ID(UAFire) || input.SyncedClick_ID(UAFire))
				isAttack = true;
			if (input.SyncedValue_ID(UADefaultAction) > 0 || input.SyncedPress_ID(UADefaultAction) || input.SyncedClick_ID(UADefaultAction))
				isAttack = true;
		}
		if (!isAttack && hic)
		{
			if (hic.IsAttackButton() || hic.IsAttackButtonDown() || hic.IsUseButton() || hic.IsUseButtonDown())
				isAttack = true;
		}

		// Steering: mouse horizontal aim delta and A/D keyboard strafe
		float turnInput = 0.0;
		if (input)
		{
			float mouseX = input.SyncedValue_ID(UAAimRight) - input.SyncedValue_ID(UAAimLeft);
			float keyX = input.SyncedValue_ID(UAMoveRight) - input.SyncedValue_ID(UAMoveLeft);
			turnInput = mouseX;
			if (Math.AbsFloat(keyX) > 0.01)
				turnInput = keyX;

			if (input.SyncedValue_ID(UALookAround) > 0)
				turnInput = 0;
		}

		float maxTurnSpeed = 120.0;
		turnInput = Math.Clamp(turnInput * maxTurnSpeed * pDt, -180, 180);

		float heading = GetOrientation()[0] + turnInput;
		inputController.OverrideTurnSpeed(true, Math.PI2 / pDt);
		inputController.OverrideHeading(true, heading * Math.DEG2RAD);

		// Movement forward / backward / sprint
		bool isMoveFwd = false;
		bool isMoveBack = false;
		bool isSprint = false;
		bool isJog = false;

		if (input)
		{
			isMoveFwd = input.SyncedValue_ID(UAMoveForward) > 0;
			isMoveBack = input.SyncedValue_ID(UAMoveBack) > 0;
			isSprint = input.SyncedValue_ID(UATurbo) > 0;
			isJog = input.SyncedValue_ID(UAWalkRunTemp) > 0;
		}

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

		// Jump / Vault (Spacebar)
		if (input && input.SyncedValue_ID(UAGetOver) > 0)
		{
			inputController.OverrideJump(true, 101, 1.5);
		}

		// Trigger Melee Attack
		if (isAttack && m_AttackCooldownTime <= 0)
		{
			EntityAI target = ZV_FindTargetInFront(3.5, 60.0);
			m_ActualTarget = target;

			float dist = 1.5;
			int pitch = 0;
			if (target)
			{
				dist = vector.Distance(target.GetPosition(), GetPosition());
				pitch = GetAttackPitch(target);
			}

			int animGroup = DayZInfectedAttackGroupType.FIGHT;
			if (fSpeed > 0)
				animGroup = DayZInfectedAttackGroupType.CHASE;

			m_ActualAttackType = GetDayZInfectedType().ChooseAttack(animGroup, dist, pitch);
			if (!m_ActualAttackType)
				m_ActualAttackType = GetDayZInfectedType().ChooseAttack(DayZInfectedAttackGroupType.FIGHT, dist, pitch);
			if (!m_ActualAttackType)
				m_ActualAttackType = GetDayZInfectedType().ChooseAttack(DayZInfectedAttackGroupType.CHASE, dist, pitch);

			int animType = 0;
			float subType = 0.0;
			if (m_ActualAttackType)
			{
				animType = m_ActualAttackType.m_Type;
				subType = m_ActualAttackType.m_Subtype;
				m_AttackCooldownTime = m_ActualAttackType.m_Cooldown;
			}
			else
			{
				animType = Math.RandomInt(0, 3);
				subType = 0.0;
				m_AttackCooldownTime = 0.8;
			}

			StartCommand_Attack(m_ActualTarget, animType, subType);
			return true;
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
