from build import *
objects=compile_objects('probe',[TOOLS/'probe.cpp'],[GEN,SRC],['UNICODE','_UNICODE'])
run_tool('link.exe',['/nologo','/DYNAMICBASE','/NXCOMPAT','/GUARD:CF','/OUT:'+quote(OUT/'probe.exe')]+list(map(quote,objects))+['windowscodecs.lib','shlwapi.lib','ole32.lib','uuid.lib'],'probe-link')
