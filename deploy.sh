sudo cp joystick2mqtt.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl restart joystick2mqtt
sudo systemctl status joystick2mqtt

