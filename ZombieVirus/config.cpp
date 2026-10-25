class CfgPatches
{
	class ZombieVirus
	{
		units[] = {"ZV_Antiviral"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Medical", "DZ_Characters_Zombies"};
	};
};

class CfgMods
{
	class ZombieVirus
	{
		dir = "ZombieVirus";
		name = "Zombie Virus";
		credits = "GhostHex";
		author = "GhostHex";
		version = "1.0";
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};
		class defs
		{
			class gameScriptModule { value = ""; files[] = {"ZombieVirus/scripts/3_Game"}; };
			class worldScriptModule { value = ""; files[] = {"ZombieVirus/scripts/4_World"}; };
			class missionScriptModule { value = ""; files[] = {"ZombieVirus/scripts/5_Mission"}; };
		};
	};
};

class CfgVehicles
{
	class AntiChemInjector;
	// Same model and inject actions as the vanilla antidote injector, green Z-strain label.
	class ZV_Antiviral: AntiChemInjector
	{
		scope = 2;
		displayName = "Z-Strain Antiviral";
		descriptionShort = "Experimental antiviral autoinjector. Destroys the Z-strain virus in the blood if used before the infection runs its course. Injected while healthy, it gives a short period of immunity.";
		hiddenSelectionsTextures[] = {"ZombieVirus\data\zv_antiviral_co.paa"};
	};
};
