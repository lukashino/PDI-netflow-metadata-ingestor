from time import sleep
from json import dumps
from kafka import KafkaProducer

producer = KafkaProducer(bootstrap_servers=['51.116.188.112:9092'],value_serializer=lambda x:dumps(x).encode('utf-8'))

for e in range(1000):
    # data = {'number' : e}
    producer.send('pdi', value="cau")
    sleep(2)
    print('sent')
