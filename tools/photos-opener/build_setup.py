from build import *
digest=hashlib.sha256((OUT/'wic_heic_local.dll').read_bytes()).hexdigest()
opener_digest=hashlib.sha256((OUT/'WicHeicOpen.exe').read_bytes()).hexdigest()
(GEN/'release_hash.h').write_text('#define EXPECTED_DLL_SHA256 "'+digest+'"\n#define EXPECTED_OPENER_SHA256 "'+opener_digest+'"\n',encoding='utf-8')
objects=compile_objects('setup',[TOOLS/'setup.cpp'],[GEN],['UNICODE','_UNICODE'])
run_tool('link.exe',['/nologo','/DYNAMICBASE','/NXCOMPAT','/GUARD:CF','/OUT:'+quote(OUT/'WicHeicLocalSetup.exe')]+list(map(quote,objects))+['bcrypt.lib','advapi32.lib'],'setup-link')
