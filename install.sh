echo "Installing bluetooth"
apt-get install libbluetooth-dev

echo "Install python dependencies"
apt-get update && apt-get install python-serial python-setuptools python-dev python-smbus python-pip
pip install --upgrade PyBBIO

echo "Setting up suitcontrol service"
cp ./suitcontrol.service /etc/systemd/system/suitcontrol.service
