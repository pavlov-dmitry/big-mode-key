mkdir bin
call stop.bat
pushd bin
cl /O2 ../src/big_mode_key.c ../src/config.c user32.lib
popd
