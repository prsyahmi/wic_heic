from build import *
digest=hashlib.sha256((OUT/'wic_heic_local.dll').read_bytes()).hexdigest()
(GEN/'release_hash.h').write_text('#define EXPECTED_DLL_SHA256 "'+digest+'"\n',encoding='utf-8')
objects=compile_objects('opener',[TOOLS/'opener.cpp'],[GEN,SRC],['UNICODE','_UNICODE'])
run_tool('link.exe',['/nologo','/SUBSYSTEM:WINDOWS','/DYNAMICBASE','/NXCOMPAT','/GUARD:CF','/OUT:'+quote(OUT/'WicHeicOpen.exe')]+list(map(quote,objects))+['bcrypt.lib','shell32.lib','shlwapi.lib','windowscodecs.lib','ole32.lib','uuid.lib','user32.lib'],'opener-link')
result=subprocess.run([str(OUT/'WicHeicOpen.exe'),'--self-test'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=30)
output=result.stdout.decode('utf-8',errors='replace');print(output,flush=True)
(OUT/'opener-self-test.log').write_text(output,encoding='utf-8')
if result.returncode:raise RuntimeError('Opener self-test failed: '+str(result.returncode))
