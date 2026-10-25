// Custom camera override for playable feral zombie mode.
// Directly setting m_fInsideCamera = 1.0 causes the Enforce engine to cull the player character mesh.

modded class DayZPlayerCamera3rdPerson
{
	override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		PlayerBase pb = PlayerBase.Cast(m_pPlayer);
		if (pb && pb.ZV_IsFeral())
		{
			pOutResult.m_fInsideCamera = 1.0;
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
		}
	}
}
