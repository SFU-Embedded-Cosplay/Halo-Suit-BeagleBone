hciconfig hci0 piscan
sudo hciconfig -a

echo -e "If the above message did not contain \"UP RUNNING PSCAN ISCAN\" then there is likely a problem with the bluetooth dongle.\n"
echo "Waiting for bluetooth connection:"

bluez-simple-agent
