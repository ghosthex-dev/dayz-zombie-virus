class HumanCommandScript_ZVInfectedControl : HumanCommandScript
{
	protected PlayerBase m_Player;
	protected ZombieBase m_Zombie;

	void HumanCommandScript_ZVInfectedControl(DayZPlayer pPlayer, ZombieBase pZombie)
	{
		m_Player = PlayerBase.Cast(pPlayer);
		m_Zombie = pZombie;
	}

	override void OnActivate()
	{
		if (m_Player)
			dBodyEnableGravity(m_Player, false);
	}

	override void OnDeactivate()
	{
		if (m_Player)
			dBodyEnableGravity(m_Player, true);
	}

	override void PrePhysUpdate(float pDt)
	{
		if (m_Zombie && m_Zombie.IsAlive())
		{
			SetHeading(m_Zombie.GetOrientation()[0] * Math.DEG2RAD);
		}
	}

	override bool PostPhysUpdate(float pDt)
	{
		if (!m_Zombie || !m_Zombie.IsAlive() || !m_Player || !m_Player.ZV_IsFeral())
		{
			SetFlagFinished(true);
			return false;
		}

		PostPhys_SetPosition(m_Zombie.GetPosition());
		return true;
	}
}

modded class DayZPlayerImplement
{
	override int CameraHandler(int pCameraMode)
	{
		PlayerBase pb = PlayerBase.Cast(this);
		if (pb && pb.ZV_IsFeral())
		{
			m_Camera3rdPerson = true;
			return DayZPlayerCameras.DAYZCAMERA_3RD_ERC;
		}
		return super.CameraHandler(pCameraMode);
	}

	override bool ModCommandHandlerBefore(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		PlayerBase pb = PlayerBase.Cast(this);
		if (pb && pb.ZV_IsFeral())
		{
			if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT)
			{
				HumanCommandScript hcs = GetCommand_Script();
				if (HumanCommandScript_ZVInfectedControl.Cast(hcs))
					return true;
			}

			ZombieBase zmb = pb.ZV_GetPuppetZombie();
			if (!zmb && pb.GetParent() && pb.GetParent().IsInherited(ZombieBase))
			{
				zmb = ZombieBase.Cast(pb.GetParent());
				pb.ZV_SetPuppetZombie(zmb);
			}

			if (!zmb)
			{
				array<Object> nearbyObjs = new array<Object>;
				GetGame().GetObjectsAtPosition3D(pb.GetPosition(), 10.0, nearbyObjs, null);
				for (int noi = 0; noi < nearbyObjs.Count(); noi++)
				{
					if (nearbyObjs[noi].IsInherited(ZombieBase))
					{
						zmb = ZombieBase.Cast(nearbyObjs[noi]);
						pb.ZV_SetPuppetZombie(zmb);
						break;
					}
				}
			}

			if (zmb && zmb.IsAlive())
			{
				StartCommand_Script(new HumanCommandScript_ZVInfectedControl(this, zmb));
				return true;
			}

			// Block default HumanCommandMove while waiting for puppet zombie reference
			return true;
		}

		return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);
	}
}
