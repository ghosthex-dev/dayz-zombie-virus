// AI test characters have no identity, so they would never be saved. Give them a key while the self-test runs.
class ZV_TestState
{
	static bool s_On;
}

modded class PlayerBase
{
	override string ZV_Key()
	{
		string key = super.ZV_Key();
		if (key == "" && ZV_TestState.s_On)
			return "zvtest_" + GetID();
		return key;
	}
}
