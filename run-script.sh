#!/bin/bash
set -e # exit on error code

IFACE=$1

if /sbin/ethtool "$IFACE" | grep -q "Link detected: yes"; then
    echo "$IFACE online"
else
    echo "Interface(\"$IFACE\") not specified or not active"
    echo "Specify the interface as the first argument"
    echo "\"tcpreplay\" is requirement to run the script"
    exit 1
fi

make clean && make;

echo "Run your Python consumer in separate terminal window and then press enter"
echo "python3 cons.py"
read;

# Run the producer (packet sniffer)
sudo ./prod -i $IFACE > prod.output 2>&1 &
PID=$!
echo "$PID" | sudo tee prod.pid # if script fails

# najprv pustim tcpreplay UDP s 20 paketmi, ukaze sa mu to na readerovi
echo "sudo tcpreplay -i $IFACE -l 10 samples/chargen-udp.pcap";
echo "PCAP contents:";
tcpdump -vvv -xx -XX -n -N --number -r samples/chargen-udp.pcap
echo "Press enter to play the pcap";
read;

sudo tcpreplay -i $IFACE -l 10 samples/chargen-udp.pcap

# potom pustim tcpreplay TCP s vela paketmi a ukaze sa mu na readovi iba tych prvych 20p
echo "sudo tcpreplay -i $IFACE -l 10 samples/SIMULCRYPT.pcap";
echo "PCAP contents:";
tcpdump -vvv -xx -XX -n -N --number -r samples/SIMULCRYPT.pcap
echo "Press enter to play the pcap";
read;

sudo tcpreplay -i $IFACE -l 10 -x 60.0 samples/SIMULCRYPT.pcap

# potom pustim tcpreplay UDP s 3 paketmi a --unique-ip, po timeout-e sa mu to ukaze na readerovi 
echo "sudo tcpreplay -i $IFACE -l 3 --unique-ip samples/chargen-udp.pcap";
echo "PCAP contents:";
tcpdump -vvv -xx -XX -n -N --number -r samples/chargen-udp.pcap
echo "Press enter to play the pcap";
read;

sudo tcpreplay -i $IFACE -l 3 --unique-ip samples/chargen-udp.pcap

echo "WAITING FOR THE FLOW TO END (UDP flow timeout)"
echo "Press enter to exit"
read

echo "Switching the producer off";
sudo killall -9 "./prod"