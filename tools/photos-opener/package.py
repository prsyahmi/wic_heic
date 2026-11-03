from build import *
from datetime import datetime,timezone
import zipfile

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
manifest={
    'name':'WicHeic Local','version':'1.1.17.0','architecture':'x64',
    'built_utc':datetime.now(timezone.utc).isoformat(),
    'dll_sha256':sha(OUT/'wic_heic_local.dll'),
    'opener_sha256':sha(OUT/'WicHeicOpen.exe'),
    'dependencies':json.loads((TOOLS/'dependencies.json').read_text()),
    'features':{'output':'24-bit RGB SDR','metadata':False,'encoding':False,'external_plugins':False},
    'code_signing':'unsigned development build'
}
(OUT/'release-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
with zipfile.ZipFile(OUT/'WicHeic-Photos-Opener-x64.zip','w',zipfile.ZIP_DEFLATED) as z:
    for name in ['wic_heic_local.dll','WicHeicOpen.exe','WicHeicLocalSetup.exe','probe.exe','security_tests.exe','release-manifest.json']:
        z.write(OUT/name,name)
    z.write(TOOLS/'README.md','README.md')
    z.write(ROOT/'Installer/License.txt','licenses/WicHeic.txt')
    for name,path in [('libheif',HEIF),('libde265',DE265)]:
        for license_file in path.glob('COPYING*'):
            z.write(license_file,'licenses/'+name+'-'+license_file.name)
    # Static linking: retain objects and dependency libraries for relinking.
    for p in (OUT/'obj/wic').glob('*.obj'):z.write(p,'relink/wic/'+p.name)
    for name in ['heif.lib','libde265.lib']:z.write(OUT/name,'relink/'+name)
print('Packaged '+str(OUT/'WicHeic-Photos-Opener-x64.zip'))
