cd ../limero/linux/build
make
cd ../../../joystick2mqtt/build
touch ../src/Main.cpp
make
cd ..
./build/joystick2mqtt
