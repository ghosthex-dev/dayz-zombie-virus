// Third-person over-the-shoulder camera attached to the player's controlled puppet zombie.
class ZV_InfectedCamera extends Camera
{
	protected float m_SendUpdateAcc = 0.0;
	protected Object m_CamTarget;

	void ZV_InfectedCamera()
	{
		SetEventMask(EntityEvent.FRAME);
		m_CamTarget = null;
	}

	void SetTarget(Object target)
	{
		m_CamTarget = target;
	}

	override void EOnFrame(IEntity other, float timeSlice)
	{
		OnUpdate(timeSlice);

		if (m_SendUpdateAcc > 0.8)
		{
			GetGame().UpdateSpectatorPosition(GetPosition());
			m_SendUpdateAcc = 0;
		}
		m_SendUpdateAcc += timeSlice;
	}

	void OnUpdate(float timeSlice)
	{
		ZombieBase zombie = ZombieBase.Cast(m_CamTarget);
		if (!zombie)
		{
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb)
			{
				m_CamTarget = pb.ZV_GetPuppetZombie();
				zombie = ZombieBase.Cast(m_CamTarget);
			}
		}
		if (!zombie)
			return;

		Input input = GetGame().GetInput();
		SetPosition(zombie.ModelToWorld("0 1.6 -1.8"));

		float pitchDiff = input.LocalValue("UAAimDown") - input.LocalValue("UAAimUp");
		vector newOrient = GetOrientation();
		newOrient[0] = zombie.GetOrientation()[0];
		newOrient[1] = newOrient[1] - Math.RAD2DEG * pitchDiff * timeSlice;
		if (newOrient[1] < -89)
			newOrient[1] = -89;
		if (newOrient[1] > 89)
			newOrient[1] = 89;

		SetOrientation(newOrient);
	}
}
