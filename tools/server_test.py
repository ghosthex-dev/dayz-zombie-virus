"""Local test harness: DayZServer with @ZombieVirus and the test-only @ZV_Test, which runs the server self-test.

usage: python tools/server_test.py [--keep] [--fresh] [--play] [--join]
  (no flags)  boot the server, run the self-test, print its result and stop the server
  --keep      leave the server running afterwards
  --fresh     wipe the test mission's storage and the mod's saved infections first
  --play      in-game test instead of the self-test: no @ZV_Test, PLAY_SETTINGS (every zombie hit infects,
              5 minute incubation), new characters spawn with one antiviral; the server keeps running
  --join      with --play: also start the game client (DayZ_BE.exe) connected to the server
Everything lives in _test/ (gitignored): server profile and a copy of the vanilla Chernarus mission with
ServerFiles/zv_types.xml registered. Run tools/build.py first. The first boot builds the world DataCache (minutes).
"""
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from build import ROOT, MOD, find_app, pack_pbo  # noqa: E402

TEST = os.path.join(ROOT, '_test', 'server')
PROFILE = os.path.join(TEST, 'profile')
MISSION = os.path.join(TEST, 'mpmissions', 'zvtest.chernarusplus')
TEST_MOD = os.path.join(ROOT, '_test', '@ZV_Test')
PORT = 2302
TIMEOUT = 1200
INSTANCE = 79
CLIENT_PROFILE = os.path.join(os.environ.get('LOCALAPPDATA', ''), 'DayZ')

# Overrides written to the profile's settings.json in --play mode
PLAY_SETTINGS = {
    'InfectionChance': 1.0,
    'InfectionChanceIfWounded': 1.0,
    'BlockedHitMultiplier': 1.0,
    'IncubationMinutes': 5.0,
    'CureImmunityMinutes': 1.0,
    'TurnDelaySeconds': 5.0,
}
ANTIVIRAL_LINE = '\t\t\titemEnt = itemClothing.GetInventory().CreateInInventory( "ZV_Antiviral" );  // zvtest\n'


def write_cfg(path):
    with open(path, 'w') as f:
        f.write('hostname = "ZombieVirus test";\npassword = "";\npasswordAdmin = "";\nenableWhitelist = 0;\n'
                'maxPlayers = 4;\nverifySignatures = 0;\nforceSameBuild = 1;\nBattlEye = 0;\ndisableVoN = 1;\n'
                'thirdPersonView = 1;\ndisablePersonalLight = 1;\nlightingConfig = 0;\nserverTime = "2015/6/15/12/00";\n'
                'serverTimeAcceleration = 1;\nserverTimePersistent = 0;\nguaranteedUpdates = 1;\ninstanceId = %d;\n'
                'storageAutoFix = 1;\n\nclass Missions\n{\n    class DayZ\n    {\n        template = "zvtest.chernarusplus";\n    };\n};\n'
                % INSTANCE)


def prepare_mission(srv, fresh):
    src = os.path.join(srv, 'mpmissions', 'dayzOffline.chernarusplus')
    if not os.path.isdir(MISSION):
        shutil.copytree(src, MISSION, ignore=shutil.ignore_patterns('storage_*'))
    os.makedirs(os.path.join(MISSION, 'zombievirus'), exist_ok=True)
    shutil.copy2(os.path.join(ROOT, 'ServerFiles', 'zv_types.xml'), os.path.join(MISSION, 'zombievirus', 'zv_types.xml'))
    core = os.path.join(MISSION, 'cfgeconomycore.xml')
    text = open(core, encoding='utf-8').read()
    if 'zombievirus' not in text:
        snippet = '\t<ce folder="zombievirus">\n\t\t<file name="zv_types.xml" type="types" />\n\t</ce>\n'
        text = text.replace('</economycore>', snippet + '</economycore>')
        open(core, 'w', encoding='utf-8').write(text)
    if fresh:
        shutil.rmtree(os.path.join(MISSION, 'storage_%d' % INSTANCE), ignore_errors=True)
        saved = os.path.join(PROFILE, 'ZombieVirus', 'players.json')
        if os.path.isfile(saved):
            os.remove(saved)


def set_play_mode(play):
    """Adds/removes the starting antiviral in the mission's init.c and writes/removes the fast settings."""
    init = os.path.join(MISSION, 'init.c')
    lines = [ln for ln in open(init, encoding='utf-8').readlines() if '// zvtest' not in ln]
    if play:
        for i, ln in enumerate(lines):
            if 'CreateInInventory( "BandageDressing" )' in ln:
                lines.insert(i + 1, ANTIVIRAL_LINE)
                break
    open(init, 'w', encoding='utf-8').writelines(lines)
    settings = os.path.join(PROFILE, 'ZombieVirus', 'settings.json')
    if play:
        data = {}
        if os.path.isfile(settings):
            data = json.load(open(settings, encoding='utf-8'))
        data.update(PLAY_SETTINGS)
        os.makedirs(os.path.dirname(settings), exist_ok=True)
        json.dump(data, open(settings, 'w', encoding='utf-8'), indent=4)
    elif os.path.isfile(settings):
        os.remove(settings)   # the self-test expects the mod to write the defaults itself


def join():
    game = find_app('DayZ', 'DayZ_x64.exe')
    if running('DayZ_x64.exe'):
        print('the game is already running; close it, then join 127.0.0.1:%d with -mod=%s' % (PORT, MOD))
        return
    exe = 'DayZ_BE.exe' if os.path.isfile(os.path.join(game, 'DayZ_BE.exe')) else 'DayZ_x64.exe'
    start_detached([os.path.join(game, exe), '-connect=127.0.0.1', '-port=%d' % PORT, '-mod=' + MOD,
                    '-name=ZVTester', '-nosplash', '-skipIntro', '-noBenchmark', '-window', '-nopause'], game)
    print('client started (%s)' % exe)


def build_test_mod():
    src = os.path.join(ROOT, 'tools', 'testmod', 'ZV_Test')
    tools = find_app('DayZ Tools', os.path.join('Bin', 'CfgConvert', 'CfgConvert.exe'))
    stage = os.path.join(ROOT, 'build', 'stage_test')
    shutil.rmtree(stage, ignore_errors=True)
    shutil.copytree(src, stage)
    if tools:
        cc = os.path.join(tools, 'Bin', 'CfgConvert', 'CfgConvert.exe')
        subprocess.run([cc, '-bin', '-dst', os.path.join(stage, 'config.bin'), os.path.join(stage, 'config.cpp')], capture_output=True)
        if os.path.isfile(os.path.join(stage, 'config.bin')):
            os.remove(os.path.join(stage, 'config.cpp'))
    pack_pbo(stage, 'ZV_Test', os.path.join(TEST_MOD, 'addons', 'ZV_Test.pbo'))
    shutil.rmtree(stage, ignore_errors=True)


def read_logs(folder):
    text = ''
    for p in glob.glob(os.path.join(folder, '*.RPT')) + glob.glob(os.path.join(folder, 'script*.log')):
        with open(p, encoding='utf-8', errors='replace') as f:
            text += f.read() + '\n'
    return text


ERRORS = r"Can't compile|Compile error|SCRIPT\s+\(E\)|NULL pointer"


def print_matches(logs):
    seen = set()
    for line in logs.splitlines():
        s = line.strip()
        if s in seen:
            continue
        if re.search(r"\[ZV\]|ZombieVirus|ZV_Test|Cannot open|" + ERRORS, s, re.I):
            seen.add(s)
            print('  ' + s[:300])


def running(image):
    return image in subprocess.run(['tasklist', '/FI', 'IMAGENAME eq ' + image], capture_output=True, text=True).stdout


def start_detached(args, cwd):
    flags = subprocess.CREATE_NEW_CONSOLE | subprocess.CREATE_NEW_PROCESS_GROUP | 0x01000000  # BREAKAWAY_FROM_JOB
    try:
        return subprocess.Popen(args, cwd=cwd, creationflags=flags)
    except OSError:
        return subprocess.Popen(args, cwd=cwd, creationflags=flags & ~0x01000000)


def main():
    argv = sys.argv[1:]
    play = '--play' in argv
    keep = '--keep' in argv or play
    srv = find_app('DayZServer', 'DayZServer_x64.exe')
    if not srv:
        print('DayZServer not installed')
        return 2
    if not os.path.isfile(os.path.join(MOD, 'addons', 'ZombieVirus.pbo')):
        print('run tools/build.py first')
        return 2
    if running('DayZServer_x64.exe'):
        print('a DayZServer is already running; stop it first')
        return 2
    prepare_mission(srv, '--fresh' in argv)
    set_play_mode(play)
    mods = MOD
    if not play:
        build_test_mod()
        mods = MOD + ';' + TEST_MOD
    # Fresh logs every run, but keep the DataCache: rebuilding it costs minutes per boot.
    os.makedirs(PROFILE, exist_ok=True)
    for fn in os.listdir(PROFILE):
        if fn.lower().endswith(('.rpt', '.log', '.adm', '.mdmp')):
            os.remove(os.path.join(PROFILE, fn))
    cfg = os.path.join(TEST, 'serverDZ.cfg')
    write_cfg(cfg)
    args = [os.path.join(srv, 'DayZServer_x64.exe'), '-config=' + cfg, '-port=%d' % PORT, '-profiles=' + PROFILE,
            '-mission=' + MISSION, '-mod=' + mods, '-dologs', '-adminlog', '-freezecheck']
    proc = start_detached(args, srv)

    start = time.time()
    outcome = 'timeout'
    while time.time() - start < TIMEOUT:
        time.sleep(3)
        logs = read_logs(PROFILE)
        if re.search(r"Can't compile|Compile error", logs):
            time.sleep(3)
            outcome = 'compile errors'
            break
        if play and ('Init sequence finished' in logs or 'Mission read' in logs):
            time.sleep(5)
            outcome = 'started'
            break
        if '[ZV][TEST] DONE' in logs:
            time.sleep(2)
            outcome = 'self-test finished'
            break
        if proc.poll() is not None:
            outcome = 'server exited (code %s)' % proc.returncode
            break

    logs = read_logs(PROFILE)
    print('server: %s after %.0f s' % (outcome, time.time() - start))
    print_matches(logs)
    if play:
        if outcome != 'started':
            subprocess.run(['taskkill', '/PID', str(proc.pid), '/T', '/F'], capture_output=True)
            return 1
        print('server running on 127.0.0.1:%d (pid %d) with play settings' % (PORT, proc.pid))
        if '--join' in argv:
            join()
        return 0
    m = re.search(r'\[ZV\]\[TEST\] DONE pass=(\d+) fail=(\d+)', logs)
    ok = bool(m) and m.group(2) == '0' and not re.search(ERRORS, logs)
    if keep and outcome == 'self-test finished':
        print('server running on port %d (pid %d)' % (PORT, proc.pid))
    else:
        subprocess.run(['taskkill', '/PID', str(proc.pid), '/T', '/F'], capture_output=True)
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
