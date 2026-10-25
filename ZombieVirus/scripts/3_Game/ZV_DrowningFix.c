modded class PPERequester_Drowning
{
	override protected void OnStart(Param par = null)
	{
		super.OnStart(par);
		Stop();
	}

	override protected void OnUpdate(float delta)
	{
		Stop();
	}
}
