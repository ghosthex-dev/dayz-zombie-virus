modded class PPERequester_Drowning
{
	override protected void OnStart(Param par = null)
	{
		super.OnStart(par);
		Stop();
		m_IsRunning = false;
		SetTargetValueFloat(PostProcessEffectType.Glow, PPEGlow.PARAM_VIGNETTE, false, 0.0, PPEGodRays.L_0_GLASSES, PPOperators.SET);
		SetTargetValueColor(PostProcessEffectType.Glow, PPEGlow.PARAM_VIGNETTECOLOR, {0.0, 0.0, 0.0, 0.0}, PPEGlow.L_23_GLASSES, PPOperators.SET);
	}

	override protected void OnUpdate(float delta)
	{
		Stop();
		m_IsRunning = false;
		SetTargetValueFloat(PostProcessEffectType.Glow, PPEGlow.PARAM_VIGNETTE, false, 0.0, PPEGodRays.L_0_GLASSES, PPOperators.SET);
		SetTargetValueColor(PostProcessEffectType.Glow, PPEGlow.PARAM_VIGNETTECOLOR, {0.0, 0.0, 0.0, 0.0}, PPEGlow.L_23_GLASSES, PPOperators.SET);
	}
}
