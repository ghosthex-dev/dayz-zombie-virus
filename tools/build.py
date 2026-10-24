"""Pack ZombieVirus/ into @ZombieVirus/addons/ZombieVirus.pbo (config.cpp binarized with CfgConvert when DayZ Tools is installed).
Optionally signs the PBO with a private key (DSSignFile) and copies the public .bikey into @ZombieVirus/keys/.

usage: python tools/build.py [--key PATH_TO_KEY.biprivatekey]
"""
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'ZombieVirus')
MOD = os.path.join(ROOT, '@ZombieVirus')
STAGE = os.path.join(ROOT, 'build', 'stage')
PREFIX = 'ZombieVirus'


def steam_roots():
    roots = [r'C:\Program Files (x86)\Steam']
    vdf = os.path.join(roots[0], 'steamapps', 'libraryfolders.vdf')
    if os.path.isfile(vdf):
        for line in open(vdf, encoding='utf-8', errors='replace'):
            line = line.strip()
            if line.startswith('"path"'):
                roots.append(line.split('"')[3].replace('\\\\', '\\'))
    return [r for r in dict.fromkeys(roots) if os.path.isdir(r)]


def find_app(name, probe):
    for r in steam_roots():
        p = os.path.join(r, 'steamapps', 'common', name)
        if os.path.isfile(os.path.join(p, probe)):
            return p
    return None


def pack_pbo(src_dir, prefix, out_path):
    """Uncompressed PBO: header entry with prefix, file table, data, SHA1 trailer."""
    files = []
    for root, _, names in os.walk(src_dir):
        for n in names:
            full = os.path.join(root, n)
            files.append((os.path.relpath(full, src_dir).replace('/', '\\'), full))
    files.sort(key=lambda t: t[0].lower())
    out = bytearray()
    out += b'\0' + struct.pack('<5I', 0x56657273, 0, 0, 0, 0)
    out += b'prefix\0' + prefix.encode() + b'\0' + b'\0'
    datas = []
    for rel, full in files:
        with open(full, 'rb') as f:
            d = f.read()
        datas.append(d)
        out += rel.encode() + b'\0' + struct.pack('<5I', 0, len(d), 0, 0, len(d))
    out += b'\0' + struct.pack('<5I', 0, 0, 0, 0, 0)
    for d in datas:
        out += d
    out += b'\0' + hashlib.sha1(bytes(out)).digest()
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(out)
    return len(files)


def cstr(f):
    b = bytearray()
    while True:
        c = f.read(1)
        if c in (b'', b'\0'):
            return b.decode('latin-1')
        b += c


def index_game_files(game, exts):
    """virtual path (lower case) -> (pbo, offset, size) for uncompressed PBO entries with one of `exts`."""
    idx = {}
    addons = os.path.join(game, 'Addons')
    for fn in os.listdir(addons):
        if not fn.lower().endswith('.pbo'):
            continue
        path = os.path.join(addons, fn)
        with open(path, 'rb') as f:
            prefix, entries = '', []
            while True:
                name = cstr(f)
                method, orig, _res, _ts, size = struct.unpack('<5I', f.read(20))
                if name == '' and method == 0x56657273:
                    while True:
                        k = cstr(f)
                        if not k:
                            break
                        v = cstr(f)
                        if k == 'prefix':
                            prefix = v
                    continue
                if name == '':
                    break
                entries.append((name, method, orig, size))
            off = f.tell()
        for name, method, orig, size in entries:
            if name.lower().endswith(exts) and (method == 0 or orig == size):
                vp = (prefix.strip('\\') + '\\' + name).lower().replace('/', '\\')
                idx[vp] = (path, off, size)
            off += size
    return idx


def binarize_models(tools, stage):
    """Binarize stage/data/models/*.p3d in place. Binarize resolves every texture/material path against
    its working directory (normally the P: drive), so the vanilla files our models reference are copied
    out of the local game install into build/binroot first. They are never packed into the mod."""
    models = os.path.join(stage, 'data', 'models')
    if not os.path.isdir(models):
        return 0
    game = find_app('DayZ', 'DayZ_x64.exe')
    if not game:
        print('DayZ not found: models left unbinarized')
        return 0
    root = os.path.join(ROOT, 'build', 'binroot')
    out = os.path.join(ROOT, 'build', 'binout')
    shutil.rmtree(out, ignore_errors=True)
    shutil.rmtree(os.path.join(root, PREFIX), ignore_errors=True)
    shutil.copytree(os.path.join(stage, 'data'), os.path.join(root, PREFIX, 'data'))

    refs = set()
    pat = re.compile(rb'dz\\[A-Za-z0-9_\\]+\.(?:paa|rvmat)', re.I)
    for folder in (models, os.path.join(stage, 'data', 'materials')):
        for fn in os.listdir(folder) if os.path.isdir(folder) else []:
            with open(os.path.join(folder, fn), 'rb') as f:
                refs.update(m.decode().lower() for m in pat.findall(f.read()))
    missing = [r for r in refs if not os.path.isfile(os.path.join(root, r))]
    if missing:
        idx = index_game_files(game, ('.paa', '.rvmat'))
        for r in missing:
            if r not in idx:
                print('  vanilla file not found in game data: ' + r)
                continue
            pbo, off, size = idx[r]
            dst = os.path.join(root, r)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            with open(pbo, 'rb') as f:
                f.seek(off)
                data = f.read(size)
            with open(dst, 'wb') as f:
                f.write(data)

    exe = os.path.join(tools, 'Bin', 'Binarize', 'binarize.exe')
    r = subprocess.run([exe, '-always', '-silent', '-maxProcesses=0', '-binPath=' + os.path.dirname(exe),
                        os.path.join(root, PREFIX, 'data', 'models'), out],
                       cwd=root, capture_output=True, text=True, errors='replace')
    log = r.stdout + r.stderr
    with open(os.path.join(ROOT, 'build', 'binarize.log'), 'w', encoding='utf-8') as f:
        f.write(log)
    errors = [ln.strip() for ln in log.splitlines() if 'rror' in ln or 'not loaded' in ln.lower() or 'Warning' in ln]
    done = 0
    for fn in os.listdir(models):
        src = os.path.join(out, fn)
        if fn.lower().endswith('.p3d') and os.path.isfile(src):
            with open(src, 'rb') as f:
                if f.read(4) != b'ODOL':
                    continue
            shutil.copy2(src, os.path.join(models, fn))
            done += 1
    for e in errors[:20]:
        print('  binarize: ' + e)
    return done


def rapify_materials(tools, stage):
    folder = os.path.join(stage, 'data', 'materials')
    cc = os.path.join(tools, 'Bin', 'CfgConvert', 'CfgConvert.exe')
    for fn in os.listdir(folder) if os.path.isdir(folder) else []:
        p = os.path.join(folder, fn)
        tmp = p + '.bin'
        subprocess.run([cc, '-bin', '-dst', tmp, p], capture_output=True)
        if os.path.isfile(tmp):
            os.replace(tmp, p)


def find_private_key(explicit=None):
    if explicit and os.path.isfile(explicit):
        return explicit
    for folder in (os.path.join(ROOT, 'keys'), ROOT):
        if os.path.isdir(folder):
            for fn in sorted(os.listdir(folder)):
                if fn.lower().endswith('.biprivatekey'):
                    return os.path.join(folder, fn)
    return None


def sign_pbo(tools, pbo_path, key_path=None):
    ds_sign = os.path.join(tools, 'Bin', 'DsUtils', 'DSSignFile.exe')
    if not os.path.isfile(ds_sign):
        return None
    priv = find_private_key(key_path)
    if not priv:
        return None
    key_name = os.path.splitext(os.path.basename(priv))[0]
    pbo_dir = os.path.dirname(pbo_path)
    base = os.path.basename(pbo_path)
    for fn in os.listdir(pbo_dir):
        if fn.startswith(base) and fn.lower().endswith('.bisign'):
            try:
                os.remove(os.path.join(pbo_dir, fn))
            except OSError:
                pass
    r = subprocess.run([ds_sign, priv, pbo_path], capture_output=True, text=True)
    bisign = pbo_path + '.' + key_name + '.bisign'
    if not os.path.isfile(bisign):
        print('DSSignFile failed:\n' + (r.stdout + r.stderr).strip())
        return None
    bikey = os.path.splitext(priv)[0] + '.bikey'
    if os.path.isfile(bikey):
        keys_dir = os.path.join(MOD, 'keys')
        os.makedirs(keys_dir, exist_ok=True)
        for fn in os.listdir(keys_dir):
            if fn.lower().endswith('.bikey'):
                try:
                    os.remove(os.path.join(keys_dir, fn))
                except OSError:
                    pass
        shutil.copy2(bikey, os.path.join(keys_dir, os.path.basename(bikey)))
    return key_name


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    explicit_key = None
    for i, a in enumerate(argv):
        if a == '--key' and i + 1 < len(argv):
            explicit_key = argv[i + 1]

    shutil.rmtree(STAGE, ignore_errors=True)
    shutil.copytree(SRC, STAGE, ignore=shutil.ignore_patterns('mod.cpp'))
    tools = find_app('DayZ Tools', os.path.join('Bin', 'CfgConvert', 'CfgConvert.exe'))
    cfg = os.path.join(STAGE, 'config.cpp')
    binarized = False
    if tools:
        n_models = binarize_models(tools, STAGE)
        print('binarized %d models' % n_models)
        rapify_materials(tools, STAGE)
        cc = os.path.join(tools, 'Bin', 'CfgConvert', 'CfgConvert.exe')
        r = subprocess.run([cc, '-bin', '-dst', os.path.join(STAGE, 'config.bin'), cfg], capture_output=True, text=True)
        if os.path.isfile(os.path.join(STAGE, 'config.bin')):
            os.remove(cfg)
            binarized = True
        else:
            print('CfgConvert FAILED:\n' + (r.stdout + r.stderr).strip())
            return 1
    pbo_path = os.path.join(MOD, 'addons', PREFIX + '.pbo')
    n = pack_pbo(STAGE, PREFIX, pbo_path)
    shutil.copy2(os.path.join(SRC, 'mod.cpp'), os.path.join(MOD, 'mod.cpp'))
    shutil.rmtree(STAGE, ignore_errors=True)
    key_signed = None
    if tools:
        key_signed = sign_pbo(tools, pbo_path, explicit_key)
    sign_str = (', signed with ' + key_signed + ' (public key in @ZombieVirus/keys/)') if key_signed else ', unsigned'
    print('built %s (%d files, config %s%s)' % (pbo_path, n, 'binarized' if binarized else 'text', sign_str))
    return 0


if __name__ == '__main__':
    sys.exit(main())
