// Test-only companion mod for ZombieVirus. Never ship this.
// Runs the server self-test (ZV_SelfTest.c) on mission start and logs "[ZV][TEST] DONE pass=N fail=M".
class CfgPatches
{
	class ZV_Test
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"ZombieVirus"};
	};
};

class CfgMods
{
	class ZV_Test
	{
		dir = "ZV_Test";
		name = "ZombieVirus self-test";
		type = "mod";
		dependencies[] = {"World", "Mission"};
		class defs
		{
			class worldScriptModule { value = ""; files[] = {"ZV_Test/scripts/4_World"}; };
			class missionScriptModule { value = ""; files[] = {"ZV_Test/scripts/5_Mission"}; };
		};
	};
};
