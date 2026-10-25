// Custom camera override for playable feral zombie mode.
// Ensures 3rd person perspective stays at distance behind the puppet zombie without culling the zombie model.

modded class DayZPlayerCamera3rdPerson
{
	override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		PlayerBase pb = PlayerBase.Cast(m_pPlayer);
		if (pb && pb.ZV_IsFeral())
		{
			pOutResult.m_fInsideCamera = 0.0;
			pOutResult.m_fDistance = 2.4;

			ZombieBase zmb = pb.ZV_GetPuppetZombie();
			if (!zmb && pb.GetParent() && pb.GetParent().IsInherited(ZombieBase))
				zmb = ZombieBase.Cast(pb.GetParent());

			if (zmb)
			{
				vector playerTransformWS[4];
				m_pPlayer.GetTransform(playerTransformWS);
				vector zombiePosWS = zmb.GetPosition();
				zombiePosWS[1] = zombiePosWS[1] + 1.4; // Upper torso / shoulder level of zombie
				pOutResult.m_CameraTM[3] = zombiePosWS.InvMultiply4(playerTransformWS);
				pOutResult.m_CollisionIgnoreEntity = zmb;
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
			pOutResult.m_fInsideCamera = 0.0;
		}
	}
}
