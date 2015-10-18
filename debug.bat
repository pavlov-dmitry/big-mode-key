mkdir bin
call stop.bat
pushd bin
cl -Zi ../src/big_mode_key.c ../src/config.c user32.lib
popd
devenv bin/big_mode_key.exe
