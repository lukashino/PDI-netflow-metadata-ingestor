# PDI-netflow-metadata-ingestor
You have made a wise step if you are reading this.

Devs:
- Adam Masaryk (xmasar15)
- Lukáš Šišmiš (xsismi01)
- Matúš Švancár (xsvanc06)

Sniffo as network flow metadata ingestor is a piece of software that sniffs on interfaces (either on all of them or a single one) and sends data to Apache Kafka server specified in the input parameter as an IP address and a port. Kafka works on a publish-subscribe model and uses a topic to differentiate between the different streams of messages. Topic can also be specified as the input parameter. 

For our project, we decided to run Apache Kafka on a machine residing in the cloud and making it available to use from everywhere. You can see the diagram of the architecture in the presentation. Solution is highly portable so for this reason it is possible to set up virtual machine in any other IaaS provider. 

### Instructions to run Apache Kafka in Azure:
1. Login to `https://portal.azure.com/#home`
1. Create a virtual machine with Ubuntu 18.04
    1. Create a resource
    1. Search for `Ubuntu Server 18.04 LTS` and click Create
    1. Fill out the required fields, leave the defaults and create the machine
    1. After the VM is created head to `https://portal.azure.com/#home`, click on your VM and then go to Networking -> Add Inbound Rule 
    1. Create the rule with following properties `Source: any; Source port ranges: *; Destination: Any; Destination port ranges: 9092`
1. Connect to your VM over SSH
1. On the VM install Docker engine following [Docker install manual](https://docs.docker.com/engine/install/ubuntu/)
1. Install docker-compose following [docker-compose install manual](https://docs.docker.com/compose/install/)
1. According to [Confluent guide](https://docs.confluent.io/platform/current/quickstart/ce-docker-quickstart.html#step-1-download-and-start-cp-using-docker) clone the repository <br>
`git clone https://github.com/confluentinc/cp-all-in-one.git`
1. In the repository checkout to 6.0.1-post <br>
`cd cp-all-in-one && git checkout 6.0.1-post`
1. Go to cp-all-in-one directory
`cd cp-all-in-one`
1. Edit the docker-compose.yaml (Kafka configuration properties are stored in there and you can adjust them to your needs)<br>
`services.broker.environment.KAFKA_ADVERTISED_LISTENERS = PLAINTEXT://broker:29092,PLAINTEXT_HOST://<YOUR_PUBLIC_IP>:9092` 
1. After all steps you should be able to run [Confluent Apache Kafka Docker image](https://hub.docker.com/r/confluentinc/cp-kafka/) via docker-compose <br>
`docker-compose up -d` 
1. To verify container is running execute <br>
`docker-compose ps`

If you have successfully completed the tutorial your Kafka should be running now.

### To get you started on the packet sniffer:
- install librdkafka library<br>
`sudo apt-get install -y librdkafka-dev`
- compile the program<br>
`make`
- run the program<br>
`./sniffo [-i "ethernet-interface(eth0)"|default(all)] [-b "IP address and port of the Kafka broker"|default(51.116.188.112:9092)] [-t "Kafka topic"|default(pdi)]`

To be able to see what Sniffo sends to Kafka, you need to run Kafka reader, which essentially prints out all the information that Kafka receives on the particular topic. 

### To run the reader:
- Install kafka-python library<br>
`pip3 install kafka-python`
- Run the reader<br>
`python3 reader.py [-b "IP address and port of the Kafka broker"|default(51.116.188.112:9092)] [-t "Kafka topic"|default(pdi)]`

## run-script.sh
A test script was prepared to show all the crucial abilities of the Sniffo. It consists of three test cases that are executed in row:

1. testcase: Pcap chargen-udp.pcap consisting of 2 UDP packets is replayed 10 times on the selected interface. This means a flow should appear on the reader.py immediately as packet flow reaches 20 packets. Testcase shows the ability to handle UDP packets.
1. testcase: Pcap SIMULCRYPT.pcap consisting of 90 TCP packets is replayed 3 times on the selected interface. 6 flows should appear on the reader.py as soon as the replay is finished as number of packets reaches at least 20 packets in every flow displayed. Testcase shows the ability to handle TCP packets.
1. testcase: Pcap chargen-udp.pcap is replayed again for 3 times together with the parameter --unique-ip. This means the the first replay leaves the pcap untouched leading to never displaying on the reader. The second and the third replay modifies one of the IP addresses and thus creates each time a new flow. As it is replayed only 3 times the total number of packets is 6. The flows are shown on the reader later, after the timeout passes and they are flagged as finished.

PCAPs were downloaded from https://wiki.wireshark.org/SampleCaptures and are under GNU General Public License version 2.

Running the test script:
- install tcpreplay<br>
`sudo apt-get install -y tcpreplay`
- run the script:<br>
`./run-script [ethernet-interface(to replay on)]`
