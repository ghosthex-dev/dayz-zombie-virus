modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		ZV_Virus.Get();
		Print("[ZV] Zombie Virus loaded");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		ZV_Virus.Get().OnUpdate(timeslice);
	}

	// A brand-new character never inherits the infection of the previous one.
	override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
	{
		if (identity)
			ZV_Virus.Get().Forget(identity.GetId());
		return super.OnClientNewEvent(identity, pos, ctx);
	}

	override void OnMissionFinish()
	{
		ZV_Virus.Get().SaveAll();
		super.OnMissionFinish();
	}
}
