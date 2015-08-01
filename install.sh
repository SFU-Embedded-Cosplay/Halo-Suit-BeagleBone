echo "Installing bluetooth"
apt-get install libbluetooth-dev
apt-get install bluez

echo "Install python dependencies"
apt-get update && apt-get install python-serial python-setuptools python-dev python-smbus python-pip
pip install --upgrade PyBBIO

