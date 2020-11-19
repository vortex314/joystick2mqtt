echo "run from workspace directory !"
#  INSTALL OPENSSL dev libraries first
# sudo apt-get install libssl-dev
#
sudo apt-get install libssl-dev
git clone https://github.com/bblanchon/ArduinoJson.git
git clone https://github.com/eclipse/paho.mqtt.c.git
git clone https://github.com/vortex314/Common
git clone https://github.com/vortex314/joystick2mqtt
cd Common
make -f Common.mk
cd ../paho.mqtt.c
make
cp ../serial2mqtt/makePaho.sh .
./makePaho.sh 
cd ../joystick2mqtt
#
# change all paths in the .mk file to your paths
#
make -f joystick2mqtt.mk
