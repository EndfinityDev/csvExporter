::echo "Build Event"
echo %1
py ..\unpack_libs.py %1
node ..\update_version.js %1