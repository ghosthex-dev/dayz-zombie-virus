// Custom camera override for playable feral zombie mode.
// Modding DayZPlayerCamera directly ensures the engine's native camera pipeline
// drives the view seamlessly without relying on spectator switching.

modded class DayZPlayerCamera3rdPerson
{
	override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		PlayerBase pb = PlayerBase.Cast(m_pPlayer);
		if (pb && pb.ZV_IsFeral())
		{
			// m_fInsideCamera = 1.0 tells the DayZ engine to cull the player character mesh completely
			pOutResult.m_fInsideCamera = 1.0;

			ZombieBase zmb = pb.ZV_GetPuppetZombie();
			if (!zmb && pb.GetParent() && pb.GetParent().IsInherited(ZombieBase))
				zmb = ZombieBase.Cast(pb.GetParent());

			if (zmb)
			{
				vector camPos = zmb.ModelToWorld("0 1.7 -2.2");
				vector camAngles = GetCurrentOrientation();
				camAngles[0] = zmb.GetOrientation()[0]; // Lock yaw to zombie heading
				Math3D.YawPitchRollMatrix(camAngles, pOutResult.m_CameraTM);

				pOutResult.m_CameraTM[3] = camPos;
				pOutResult.m_fDistance = 0.0;
				pOutResult.m_fUseHeading = 0.0;
			}
		}
	}
}

modded class DayZPlayerCamera1stPerson
{
	override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		PlayerBase pb = PlayerBase.Cast(m_pPlayer);
		if (pb && pb.ZV_IsFeral())
		{
			pOutResult.m_fInsideCamera = 1.0;

			ZombieBase zmb = pb.ZV_GetPuppetZombie();
			if (!zmb && pb.GetParent() && pb.GetParent().IsInherited(ZombieBase))
				zmb = ZombieBase.Cast(pb.GetParent());

			if (zmb)
			{
				vector camPos = zmb.ModelToWorld("0 1.7 -2.2");
				vector camAngles = GetCurrentOrientation();
				camAngles[0] = zmb.GetOrientation()[0];
				Math3D.YawPitchRollMatrix(camAngles, pOutResult.m_CameraTM);

				pOutResult.m_CameraTM[3] = camPos;
				pOutResult.m_fDistance = 0.0;
				pOutResult.m_fUseHeading = 0.0;
			}
		}
	}
}
