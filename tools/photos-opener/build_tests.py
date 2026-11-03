from build import *
objects=compile_objects('security-tests',[TOOLS/'security_tests.cpp',SRC/'HeifStreamReader.cpp'],[GEN,SRC],['X64','UNICODE','_UNICODE'])
run_tool('link.exe',['/nologo','/DYNAMICBASE','/NXCOMPAT','/GUARD:CF','/OUT:'+quote(OUT/'security_tests.exe')]+list(map(quote,objects))+['windowscodecs.lib','shlwapi.lib','ole32.lib','uuid.lib'],'security-tests-link')
subprocess.run([str(OUT/'security_tests.exe'),str(OUT/'wic_heic_local.dll')],check=True,timeout=60)
