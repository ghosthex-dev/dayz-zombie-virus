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

	override bool ModCommandHandlerAfter(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		m_ZV_AIAgent = GetAIAgent();

		DayZCreatureAIInputController controller;
		GetGame().GameScript.CallFunction(this, "GetInputController", controller, 0);
		DayZInfectedInputController inputController = DayZInfectedInputController.Cast(controller);

		if (!m_ZV_Driver)
		{
			if (m_ZV_AIAgent)
				m_ZV_AIAgent.SetKeepInIdle(false);
			return super.ModCommandHandlerAfter(pDt, pCurrentCommandID, pCurrentCommandFinished);
		}

		if (m_ZV_AIAgent)
			m_ZV_AIAgent.SetKeepInIdle(true);

		PlayerBase driverPB = PlayerBase.Cast(m_ZV_Driver);
		if (!driverPB || !driverPB.IsAlive())
		{
			m_ZV_Driver = null;
			SetHealth("", "", 0);
			return true;
		}

		HumanInputController hic = driverPB.GetInputController();
		if (!hic || !inputController)
			return true;

		float headingAngle = hic.GetHeadingAngle();
		inputController.OverrideHeading(true, headingAngle);
		inputController.OverrideTurnSpeed(true, Math.PI2 / pDt);

		float plrSpeed;
		vector plrDir;
		hic.GetMovement(plrSpeed, plrDir);
		inputController.OverrideMovementSpeed(true, plrSpeed);

		// Left click / melee attack
		if (hic.IsUseButton())
		{
			EntityAI target = ZV_FindTargetInFront(7.0, 50.0);
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
					StartCommand_Attack(m_ActualTarget, m_ActualAttackType.m_Type, 2);
				}
			}
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
