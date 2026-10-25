// Third-person over-the-shoulder camera attached to the player's controlled puppet zombie.
class ZV_InfectedCamera extends Camera
{
	protected float m_SendUpdateAcc = 0.0;
	protected Object m_CamTarget;

	void ZV_InfectedCamera()
	{
		SetEventMask(EntityEvent.FRAME);
		m_CamTarget = null;
		SetActive(true);
		Print("[ZV] ZV_InfectedCamera initialized and SetActive(true)");
	}

	void ~ZV_InfectedCamera()
	{
		SetActive(false);
		Print("[ZV] ZV_InfectedCamera destroyed and SetActive(false)");
	}

	void SetTarget(Object target)
	{
		m_CamTarget = target;
		Print("[ZV] ZV_InfectedCamera SetTarget: " + target);
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
				if (pb.GetParent() && pb.GetParent().IsInherited(ZombieBase))
					m_CamTarget = pb.GetParent();
				else
					m_CamTarget = pb.ZV_GetPuppetZombie();
				zombie = ZombieBase.Cast(m_CamTarget);
			}
		}
		if (!zombie)
			return;

		Input input = GetGame().GetInput();
		
		// Camera placed directly behind the zombie: 0 X, 1.7 Y (eye level), -2.2 Z (behind)
		SetPosition(zombie.ModelToWorld("0 1.7 -2.2"));

		float pitchDiff = input.LocalValue("UAAimDown") - input.LocalValue("UAAimUp");
		vector newOrient = GetOrientation();
		newOrient[0] = zombie.GetOrientation()[0]; // Heading locked to zombie's facing direction
		newOrient[1] = Math.Clamp(newOrient[1] - Math.RAD2DEG * pitchDiff * timeSlice, -80, 80);
		newOrient[2] = 0;

		SetOrientation(newOrient);
	}
}
