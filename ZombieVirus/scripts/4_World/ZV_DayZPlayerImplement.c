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
}
