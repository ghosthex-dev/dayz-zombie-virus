# Zombie Virus (DayZ mod)

A zombie hit can infect you with the **Z-strain virus**. The infection gets worse over time. If you don't
inject a **Z-Strain Antiviral** before it runs its course, you die and get back up as one of them.

## How it plays

| Stage | Infection progress* | What happens |
|---|---|---|
| 0, Incubation | 0–25 % | No symptoms. The player gets a message that the bite burns. |
| 1, Fever | 25–50 % | Occasional coughing and sneezing. |
| 2, Severe | 50–80 % | Coughing, vomiting (empties the stomach), light pain. |
| 3, Terminal | 80–100 % | Heavy pain, vomiting, manic laughter, shaking, blurred fever vision, slow health drain. |
| Turned | 100 % | The player turns into a **playable feral zombie**! Hunt living survivors with lethal claw melee attacks, sprint with unlimited stamina, feed on raw meat/flesh to heal, and move unnoticed among wild infected hordes. (Can be toggled back to AI rise in settings). |

\* By default the whole course takes 60 minutes of **online** time. The infection pauses while the player is logged out.

While infected, a **biohazard icon with a Z** sits in the row of vanilla status icons (bottom right). Its colour
shows the stage: grey for 0, yellow for 1, orange for 2 and red for 3. It disappears when you're cured.
The stage messages use the same icon.

- **Catching it:** each zombie hit that does damage has an 8 % chance to infect. A hit that also opens a
  bleeding wound has a 25 % chance. Hits taken while blocking count at a quarter of that. A blocked light
  attack does no damage, so it can't infect.
- **Dying while infected:** a player who dies of anything else at stage 2 or later also rises.
- **The body:** the corpse stays next to the new zombie with all its gear, so friends can recover it.
- **Antibiotics** (tetracycline etc.) don't cure it. While they're active, the virus progresses at half speed.
- **Cure:** the *Z-Strain Antiviral* works at any stage, self-injected or used on another player, and gives
  30 minutes of immunity. Injected while healthy it acts as a vaccine. It uses the vanilla antidote
  autoinjector model with a green label. It spawns as rare loot in medical and military spots.

## Installing on a server

1. Load `@ZombieVirus` on the **server and every client** (`-mod=@ZombieVirus`). It needs no other mods.
2. Copy `ServerFiles/zv_types.xml` into the mission folder, for example
   `mpmissions/dayzOffline.chernarusplus/zombievirus/zv_types.xml`, and register it in `cfgeconomycore.xml`:
   ```xml
   <ce folder="zombievirus">
       <file name="zv_types.xml" type="types" />
   </ce>
   ```
3. Start the server once. It writes `<server profile>/ZombieVirus/settings.json` with the defaults. Edit that
   file and restart the server to tune the mod.

## Settings (`ZombieVirus/settings.json`)

| Key | Default | Meaning |
|---|---|---|
| `InfectionChance` | 0.08 | Chance per damaging zombie hit (0–1) |
| `InfectionChanceIfWounded` | 0.25 | Chance when that hit also caused bleeding |
| `BlockedHitMultiplier` | 0.25 | Multiplier for hits taken while blocking |
| `IncubationMinutes` | 60 | Online minutes from infection to turning |
| `Stage1At` / `Stage2At` / `Stage3At` | 0.25 / 0.5 / 0.8 | Progress where each stage begins |
| `AntibioticsSpeedMultiplier` | 0.5 | Virus speed while antibiotics are active (1 = no effect) |
| `CureImmunityMinutes` | 30 | Immunity after an antiviral injection |
| `Stage3HealthDrainPerMinute` | 4 | Health lost per minute in stage 3 (players have 100) |
| `TurnDelaySeconds` | 5 | Time between death and the zombie standing up |
| `RiseOnDeathMinStage` | 2 | Infected players who die of other causes at this stage or later also rise (4 = never) |
| `DeleteBodyOnTurn` | false | Remove the corpse when the zombie rises (deletes the gear too) |
| `NotifyPlayer` | true | Show the stage messages |
| PlayableZombie | true | Turn into a playable feral zombie upon turning (false = classic AI rise) |
| PlayableZombieDurationMinutes | 15 | Feral zombie duration before collapsing (0 = unlimited) |
| PlayableZombieIgnoredByAI | true | Wild AI zombies ignore player-controlled zombies (horde camouflage) |
| PlayableZombieInfiniteStamina | true | Unlimited sprint stamina while in feral form |
| PlayableZombieDamageMultiplier | 2.0 | Melee claw attack damage multiplier against survivors |
| PlayableZombieHealthDrainPerMinute | 2.0 | Passive health decay per minute while feral |
| PlayableZombieHealthGainOnFeed | 25.0 | Health restored when consuming raw meat or survivor flesh |
| `MaleZombies` / `FemaleZombies` | vanilla classes | Zombie classes that a turned male or female player becomes |

## Saved state

Infections are saved in `<server profile>/ZombieVirus/players.json`, keyed by player ID. The file is written
every 30 s, on logout and on shutdown. It is **not** stored in the character save, so adding or removing
the mod never corrupts characters. An entry is removed when the character dies or a new character is created.
To cure someone by hand, stop the server and delete their entry.

## Building and testing

Requires DayZ, DayZ Tools and DayZServer from Steam.

```bash
python tools/build.py
```
Packs `ZombieVirus/` into `@ZombieVirus/addons/ZombieVirus.pbo`, with `config.cpp` binarized.
If a private key is found in `keys/` (or passed via `--key`), the PBO is automatically signed
(`ZombieVirus.pbo.<author>.bisign`) and the public key (`.bikey`) is copied into `@ZombieVirus/keys/`.

```bash
python tools/server_test.py --fresh
```
Boots a local DayZServer with the mod and the test-only `@ZV_Test`. The test server always runs the
shipped default settings. The test spawns AI survivors and walks
the virus through every path: infection rolls, stages, health drain, cure and immunity, save and restore,
turning, and rising after death. It prints `[ZV][TEST] PASS/FAIL` lines. Add `--keep` to leave the server
running.

```bash
python tools/server_test.py --play --fresh --join
```
In-game test instead of the self-test. The test server gets fast settings that are written only to its own
profile (every zombie hit infects, 5-minute incubation, 1-minute immunity), and new characters start with one
antiviral. `--join` also starts the game connected to it. Running the self-test again puts the test server
back on the defaults.

`tools/make_texture.py` regenerates the antiviral texture from the vanilla antidote texture, and
`tools/make_icon.py` redraws the biohazard icon. Their outputs (`ZombieVirus/data/*.paa`) are already committed.

## Layout

- `ZombieVirus/config.cpp`: mod definition and the `ZV_Antiviral` item
- `ZombieVirus/scripts/4_World/`: `ZV_Settings`, `ZV_Virus` (settings, saving, spawning the zombie), `ZV_PlayerBase` (the infection itself), `ZV_Antiviral`
- `ZombieVirus/scripts/5_Mission/ZV_MissionServer.c`: server hooks (update, new character, shutdown)
- `ZombieVirus/scripts/5_Mission/ZV_IngameHud.c` + `gui/layouts/zv_badge.layout`: the HUD icon
- `ServerFiles/zv_types.xml`: loot entry
- `tools/`: build, texture and test scripts; `tools/testmod/ZV_Test` is the self-test mod (never ship it)
