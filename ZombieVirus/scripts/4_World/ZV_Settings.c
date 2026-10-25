// Server settings, read from $profile:ZombieVirus/settings.json (written with these defaults on first start).
class ZV_Settings
{
	int Version = 1;

	// Chance to catch the virus from one zombie hit that did damage (0..1)
	float InfectionChance = 0.08;
	// Chance instead of the above when that hit also opened a bleeding wound
	float InfectionChanceIfWounded = 0.25;
	// Multiplier for hits taken while blocking
	float BlockedHitMultiplier = 0.25;

	// Online minutes from infection until the player turns
	float IncubationMinutes = 60;
	// Infection progress (0..1) at which stages 1, 2 and 3 begin. Stage 0 has no symptoms.
	float Stage1At = 0.25;
	float Stage2At = 0.5;
	float Stage3At = 0.8;

	// The virus progresses at this speed while antibiotics are active (1 = no effect)
	float AntibioticsSpeedMultiplier = 0.5;
	// Minutes of immunity after an antiviral injection
	float CureImmunityMinutes = 30;
	// Health lost per minute in stage 3 (a player has 100)
	float Stage3HealthDrainPerMinute = 4;

	// Seconds between the player dying and the zombie standing up
	float TurnDelaySeconds = 5;
	// A player who dies of anything else while at this stage or later also rises (0..3, 4 = never)
	int RiseOnDeathMinStage = 2;
	// Delete the body when the zombie rises. Off by default so the gear stays lootable.
	bool DeleteBodyOnTurn = false;

	// Show stage messages to the infected player
	bool NotifyPlayer = true;

	// Playable Zombie mode: if true, turning keeps the player alive as a feral infected instead of dying immediately.
	// If false, the original behavior (player dies, AI zombie rises) is used.
	bool PlayableZombie = true;
	// Minutes a feral player can survive before the body finally collapses (0 = unlimited).
	float PlayableZombieDurationMinutes = 15;
	// AI zombies ignore turned players (true = horde camouflage).
	bool PlayableZombieIgnoredByAI = true;
	// Infinite sprint stamina for turned players (runner zombie speed).
	bool PlayableZombieInfiniteStamina = true;
	// Damage multiplier for feral melee attacks against survivors.
	float PlayableZombieDamageMultiplier = 2.0;
	// Health lost per minute while in feral state (starvation / decay).
	float PlayableZombieHealthDrainPerMinute = 2.0;
	// Health restored when consuming raw meat, guts, or survivor flesh.
	float PlayableZombieHealthGainOnFeed = 25.0;

	ref array<string> MaleZombies = new array<string>;
	ref array<string> FemaleZombies = new array<string>;

	// Not done in the constructor: the JSON loader fills arrays of an already constructed object.
	void FillDefaultZombies()
	{
		MaleZombies.Clear();
		FemaleZombies.Clear();
		MaleZombies.Insert("ZmbM_HermitSkinny_Beige");
		MaleZombies.Insert("ZmbM_CitizenASkinny_Blue");
		MaleZombies.Insert("ZmbM_CitizenBFat_Red");
		MaleZombies.Insert("ZmbM_HikerSkinny_Green");
		MaleZombies.Insert("ZmbM_Jacket_black");
		MaleZombies.Insert("ZmbM_MechanicSkinny_Grey");
		MaleZombies.Insert("ZmbM_HandymanNormal_Blue");
		MaleZombies.Insert("ZmbM_VillagerOld_Blue");
		FemaleZombies.Insert("ZmbF_CitizenANormal_Blue");
		FemaleZombies.Insert("ZmbF_HikerSkinny_Grey");
		FemaleZombies.Insert("ZmbF_SurvivorNormal_Red");
		FemaleZombies.Insert("ZmbF_JournalistNormal_Green");
		FemaleZombies.Insert("ZmbF_ShortSkirt_grey");
		FemaleZombies.Insert("ZmbF_VillagerOld_White");
	}

	// Keeps hand-edited values in a sane range.
	void Validate()
	{
		InfectionChance = Math.Clamp(InfectionChance, 0, 1);
		InfectionChanceIfWounded = Math.Clamp(InfectionChanceIfWounded, 0, 1);
		BlockedHitMultiplier = Math.Clamp(BlockedHitMultiplier, 0, 1);
		if (IncubationMinutes < 0.1)
			IncubationMinutes = 0.1;
		Stage1At = Math.Clamp(Stage1At, 0, 1);
		Stage2At = Math.Clamp(Stage2At, Stage1At, 1);
		Stage3At = Math.Clamp(Stage3At, Stage2At, 1);
		if (AntibioticsSpeedMultiplier < 0)
			AntibioticsSpeedMultiplier = 0;
		if (CureImmunityMinutes < 0)
			CureImmunityMinutes = 0;
		if (Stage3HealthDrainPerMinute < 0)
			Stage3HealthDrainPerMinute = 0;
		if (TurnDelaySeconds < 0)
			TurnDelaySeconds = 0;
		if (PlayableZombieDurationMinutes < 0)
			PlayableZombieDurationMinutes = 0;
		if (PlayableZombieDamageMultiplier < 0.5)
			PlayableZombieDamageMultiplier = 0.5;
		if (PlayableZombieHealthDrainPerMinute < 0)
			PlayableZombieHealthDrainPerMinute = 0;
		if (PlayableZombieHealthGainOnFeed < 0)
			PlayableZombieHealthGainOnFeed = 0;
		if (!MaleZombies)
			MaleZombies = new array<string>;
		if (!FemaleZombies)
			FemaleZombies = new array<string>;
		if (MaleZombies.Count() == 0 || FemaleZombies.Count() == 0)
			FillDefaultZombies();
	}
}
