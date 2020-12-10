#!/bin/bash
set -e # exit on error code

IFACE=$1

if /sbin/ethtool "$IFACE" | grep -q "Link detected: yes"; then
    echo "$IFACE online"
else
    echo "Interface(\"$IFACE\") not specified or not active"
    echo "Specify the interface as the first argument"
    exit 1
fi

which tcpreplay > /dev/null 2>&1
if [ "$?" != "0" ]; then
    echo "tcpreplay is required to run the script"
    echo "sudo apt-get install -y tcpreplay"
    exit 1
fi

make clean && make;

echo "Run your Python reader in a separate terminal window and then press enter"
echo "python3 reader.py"
read;

# Run the sniffo (packet sniffer)
sudo ./sniffo -i $IFACE > sniffo.output 2>&1 &
PID=$!
echo "$PID" | sudo tee sniffo.pid # if script fails

# TESTCASE 1
echo "sudo tcpreplay -i $IFACE -l 10 samples/chargen-udp.pcap";
echo "PCAP contents:";
tcpdump -vvv -xx -XX -n -N --number -r samples/chargen-udp.pcap
echo "Press enter to play the pcap";
read;

sudo tcpreplay -i $IFACE -l 10 samples/chargen-udp.pcap

# TESTCASE 2
echo "sudo tcpreplay -i $IFACE -l 10 samples/SIMULCRYPT.pcap";
echo "PCAP contents:";
tcpdump -vvv -xx -XX -n -N --number -r samples/SIMULCRYPT.pcap
echo "Press enter to play the pcap";
read;

sudo tcpreplay -i $IFACE -l 10 -x 60.0 samples/SIMULCRYPT.pcap

# TESTCASE 3
echo "sudo tcpreplay -i $IFACE -l 3 --unique-ip samples/chargen-udp.pcap";
echo "PCAP contents:";
tcpdump -vvv -xx -XX -n -N --number -r samples/chargen-udp.pcap
echo "Press enter to play the pcap";
read;

sudo tcpreplay -i $IFACE -l 3 --unique-ip samples/chargen-udp.pcap

echo "WAITING FOR THE FLOW TO END (UDP flow timeout) - see the reader"
echo "Press enter to exit"
read

echo "Switching the sniffo off";
sudo killall -9 "./sniffo"