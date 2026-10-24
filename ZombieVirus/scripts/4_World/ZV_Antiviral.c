// Uses the vanilla injector actions (self and target) inherited from AntiChemInjector; only the effect differs.
class ZV_Antiviral extends AntiChemInjector
{
	override void OnApply(PlayerBase player)
	{
		if (player)
			player.ZV_Cure();
	}
}
