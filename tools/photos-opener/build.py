"""Build the optional hardened x64 Photos opener from pinned submodules."""
from pathlib import Path
import hashlib, json, os, re, shutil, subprocess, sys

TOOLS = Path(__file__).resolve().parent
ROOT = TOOLS.parents[1]
OUT = ROOT/'build/photos-opener'
GEN = OUT/'generated'
SRC = OUT/'codec'
HEIF = ROOT/'wic_heic/3rdparty/libheif'
DE265 = ROOT/'wic_heic/3rdparty/libde265'
vswhere = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)'))/'Microsoft Visual Studio/Installer/vswhere.exe'
installation = subprocess.check_output([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip()
if not installation: raise RuntimeError('Visual Studio C++ x64 build tools were not found.')
VS = Path(installation)
def version_key(path): return tuple(int(x) for x in path.name.split('.'))
VC = max((VS/'VC/Tools/MSVC').glob('*'),key=version_key)
BIN = VC/'bin/Hostx64/x64'
SDK = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)'))/'Windows Kits/10'
SDKVER = max((p for p in (SDK/'Include').iterdir() if (p/'um/wincodec.h').exists()),key=version_key).name

def verify():
    dependencies = json.loads((TOOLS/'dependencies.json').read_text())
    for name,path in [('libheif',HEIF),('libde265',DE265)]:
        actual = subprocess.check_output(['git','-C',str(path),'rev-parse','HEAD'],text=True).strip()
        if actual != dependencies[name]['commit']:
            raise RuntimeError(name+' does not match its pinned commit; run git submodule update --init --recursive.')
    print('Verified pinned libheif and libde265 submodules.',flush=True)

def prepare():
    verify()
    OUT.mkdir(parents=True,exist_ok=True)
    GEN.mkdir(exist_ok=True)
    for folder in ('libheif', 'libde265'):
        (GEN/folder).mkdir(exist_ok=True)
    for h in (HEIF/'libheif/api/libheif').glob('*.h'):
        shutil.copy2(h, GEN/'libheif'/h.name)
    version = (HEIF/'libheif/api/libheif/heif_version.h.in').read_text()
    for k,v in {'PROJECT_VERSION_MAJOR':'1','PROJECT_VERSION_MINOR':'23','PROJECT_VERSION_PATCH':'7','PLUGIN_DIRECTORY':''}.items():
        version = version.replace('@'+k+'@',v)
    (GEN/'libheif/heif_version.h').write_text(version, encoding='utf-8')
    shutil.copy2(GEN/'libheif/heif_version.h', GEN/'heif_version.h')
    version = (DE265/'libde265/de265-version.h.in').read_text().replace('@NUMERIC_VERSION@','0x01010300').replace('@PACKAGE_VERSION@','1.1.3')
    (GEN/'libde265/de265-version.h').write_text(version, encoding='utf-8')
    for h in (DE265/'libde265').glob('*.h'):
        shutil.copy2(h, GEN/'libde265'/h.name)
    (GEN/'config.h').write_text('#pragma once\n#define HAVE_MALLOC_H 1\n', encoding='utf-8')
    SRC.mkdir(exist_ok=True)
    for p in (ROOT/'wic_heic').iterdir():
        if p.is_file() and p.suffix in ('.h','.cpp'): shutil.copy2(p,SRC/p.name)
    header = SRC/'wic_heic.h'
    text = header.read_text(encoding='utf-8-sig')
    text = text.replace('6215667c','a1c9d2a7').replace('74ca','72b6').replace('4076','4ac4')
    text = text.replace('a769-3f834c3f25f5','94f8-8a7f07c63e21')
    text = text.replace('0xa7, 0x69, 0x3f, 0x83, 0x4c, 0x3f, 0x25, 0xf5','0x94, 0xf8, 0x8a, 0x7f, 0x07, 0xc6, 0x3e, 0x21')
    header.write_text(text,encoding='utf-8')

def run_tool(tool, args, log):
    env = os.environ.copy()
    env['PATH'] = str(BIN)+os.pathsep+env.get('PATH','')
    env['INCLUDE'] = ';'.join(map(str,[VC/'include',SDK/'Include'/SDKVER/'ucrt',SDK/'Include'/SDKVER/'shared',SDK/'Include'/SDKVER/'um',SDK/'Include'/SDKVER/'winrt']))
    env['LIB'] = ';'.join(map(str,[VC/'lib/x64',SDK/'Lib'/SDKVER/'ucrt/x64',SDK/'Lib'/SDKVER/'um/x64',OUT]))
    response = OUT/(log+'.rsp')
    response.write_text('\n'.join(args), encoding='utf-8')
    result = subprocess.run([str(BIN/tool),'@'+str(response)], cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode('utf-8',errors='replace')
    (OUT/(log+'.log')).write_text(output,encoding='utf-8')
    if result.returncode:
        print('\n'.join(line for line in output.splitlines() if 'error ' in line.lower()), flush=True)
        print('\n'.join(output.splitlines()[-70:]), flush=True)
        raise RuntimeError(f'{tool}: exit {result.returncode}; see build/{log}.log')
    print(log+': passed', flush=True)

def quote(p): return '"'+str(p)+'"'

def compile_objects(name, sources, includes, defines, cpp=True):
    obj = OUT/'obj'/name
    obj.mkdir(parents=True,exist_ok=True)
    common = ['/nologo','/c','/O2','/MT','/EHsc','/GS','/guard:cf','/Z7','/utf-8','/W3','/MP4','/FS','/D_CRT_SECURE_NO_WARNINGS','/DNOMINMAX','/DNDEBUG','/D_WIN32_WINNT=0x0A00','/Fo'+quote(str(obj).replace('\\','/')+'/')]
    if cpp: common += ['/std:c++20','/Zc:__cplusplus']
    args = common+['/I'+quote(p) for p in includes]+['/D'+d for d in defines]+[quote(p) for p in sources]
    if '--resume' in sys.argv:
        pending = [p for p in sources if not (obj/(p.stem+'.obj')).exists()]
        args = common+['/I'+quote(p) for p in includes]+['/D'+d for d in defines]+[quote(p) for p in pending]
        if pending: run_tool('cl.exe',args,name+'-compile')
    else:
        run_tool('cl.exe',args,name+'-compile')
    return list(obj.glob('*.obj'))

def build_libraries():
    cmake = (DE265/'libde265/CMakeLists.txt').read_text()
    names = re.search(r'set\s*\(libde265_sources\s+(.*?)\)',cmake,re.S).group(1).split()
    sources = [DE265/'libde265'/x for x in names if x.endswith('.cc')]
    objects = compile_objects('de265', sources, [GEN,DE265,DE265/'libde265',DE265/'extra'], ['HAVE_CONFIG_H','LIBDE265_STATIC_BUILD','DE265_LOG_ERROR'])
    objects += compile_objects('de265-c',[DE265/'extra/win32cond.c'],[DE265/'extra'],[],False)
    run_tool('lib.exe',['/nologo','/OUT:'+quote(OUT/'libde265.lib')]+list(map(quote,objects)),'de265-library')
    cmake = (HEIF/'libheif/CMakeLists.txt').read_text()
    names = re.search(r'set\(libheif_sources\s+(.*?)\$\{libheif_headers\}\)',cmake,re.S).group(1).split()
    names = [x for x in names if x.endswith('.cc')]
    names += ['mini.cc','omaf_boxes.cc','plugins/encoder_mask.cc','plugins/nalu_utils.cc','plugins/decoder_libde265.cc']
    objects = compile_objects('heif',[HEIF/'libheif'/x for x in names],[GEN,HEIF,HEIF/'libheif',HEIF/'libheif/api',DE265],['LIBHEIF_STATIC_BUILD','LIBDE265_STATIC_BUILD','HAVE_LIBDE265=1','HAVE_BIT=1','ENABLE_MULTITHREADING_SUPPORT=1','IS_BIG_ENDIAN=0'])
    run_tool('lib.exe',['/nologo','/OUT:'+quote(OUT/'heif.lib')]+list(map(quote,objects)),'heif-library')

def build_wic():
    excluded = {'stdafx.cpp','HeicBitmapEncoder.cpp','HeicBitmapFrameEncode.cpp','HeifStreamWriter.cpp'}
    files = [p for p in SRC.glob('*.cpp') if p.name not in excluded]
    objects = compile_objects('wic',files,[GEN,SRC],['X64','UNICODE','_UNICODE','_WINDOWS','_USRDLL','WIC_HEIC_EXPORTS','WIC_LOCAL_DECODE_ONLY'])
    args = ['/nologo','/DLL','/DYNAMICBASE','/NXCOMPAT','/HIGHENTROPYVA','/GUARD:CF','/DEBUG','/OPT:REF','/OPT:ICF','/OUT:'+quote(OUT/'wic_heic_local.dll')]
    args += list(map(quote,objects))+[quote(OUT/'heif.lib'),quote(OUT/'libde265.lib'),'windowscodecs.lib','shlwapi.lib','ole32.lib','shell32.lib','advapi32.lib','uuid.lib']
    run_tool('link.exe',args,'wic-link')

if __name__ == '__main__':
    prepare()
    if '--prepare' in sys.argv: sys.exit(0)
    if '--wic-only' not in sys.argv: build_libraries()
    build_wic()
    print('Built '+str(OUT/'wic_heic_local.dll'),flush=True)
    if '--all' in sys.argv:
        for helper in ['build_tests.py','build_opener.py','build_probe.py','build_setup.py','package.py']:
            subprocess.run([sys.executable,str(TOOLS/helper)],check=True)
