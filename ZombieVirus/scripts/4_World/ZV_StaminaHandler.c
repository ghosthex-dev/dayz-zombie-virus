modded class StaminaHandler
{
	override void Update(float deltaT, int pCurrentCommandID)
	{
		super.Update(deltaT, pCurrentCommandID);
		if (m_Player && m_Player.ZV_IsFeral())
		{
			ZV_Settings s = ZV_Virus.Get().GetSettings();
			if (s && s.PlayableZombieInfiniteStamina)
			{
				m_Stamina = m_StaminaCap;
				m_StaminaSynced = m_StaminaCap;
				m_StaminaDepleted = false;
			}
		}
	}

	override void DepleteStaminaEx(EStaminaModifiers modifier, float dT = -1, float coef = 1.0)
	{
		if (m_Player && m_Player.ZV_IsFeral())
		{
			ZV_Settings s = ZV_Virus.Get().GetSettings();
			if (s && s.PlayableZombieInfiniteStamina)
				return;
		}
		super.DepleteStaminaEx(modifier, dT, coef);
	}
}
