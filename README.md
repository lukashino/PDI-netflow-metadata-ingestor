# PDI-netflow-metadata-ingestor
You have made a wise step if you are reading this.

Devs:
- Adam Masaryk (xmasar15)
- Lukáš Šišmiš (xsismi01)
- Matúš Švancár (xsvanc06)

To get you started on the packet sniffer:
- install librdkafka library<br>
`sudo apt-get install -y librdkafka-dev`
- compile the program<br>
`make`
- run the program<br>
`./sniffo [-i "ethernet-interface(eth0)"|default(all)] [-b "IP address and port of the Kafka broker"|default(51.116.188.112:9092)] [-t "Kafka topic"|default(pdi)]`

To be able to see what Sniffo sends to Kafka, you need to run Kafka reader, which essentially prints out all the information that Kafka receives on the particular topic. 

To run the reader:
- Install kafka-python library<br>
`pip3 install kafka-python`
- Run the reader<br>
`python3 reader.py [-b "IP address and port of the Kafka broker"|default(51.116.188.112:9092)] [-t "Kafka topic"|default(pdi)]`

## run-script.sh
A test script was prepared to show all the crucial abilities of the Sniffo. It consists of three test cases that are executed in row:

1. testcase: Pcap chargen-udp.pcap consisting of 2 UDP packets is replayed 10 times on the selected interface. This means a flow should appear on the reader.py immediately as packet flow reaches 20 packets. Testcase shows the ability to handle UDP packets.
1. testcase: Pcap SIMULCRYPT.pcap consisting of 90 TCP packets is replayed 3 times on the selected interface. 6 flows should appear on the reader.py as soon as the replay is finished as number of packets reaches at least 20 packets in every flow displayed. Testcase shows the ability to handle TCP packets.
1. testcase: Pcap chargen-udp.pcap is replayed again for 3 times together with the parameter --unique-ip. This means the the first replay leaves the pcap untouched leading to never displaying on the reader. The second and the third replay modifies one of the IP addresses and thus creates each time a new flow. As it is replayed only 3 times the total number of packets is 6. The flows are shown on the reader later, after the timeout passes and they are flagged as finished.

Running the test script:
- install tcpreplay<br>
`sudo apt-get install -y tcpreplay`
- run the script:<br>
`./run-script [ethernet-interface(to replay on)]`
