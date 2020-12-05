from kafka import KafkaConsumer
from json import loads

consumer = KafkaConsumer(
     'quickstart-events',
     bootstrap_servers=['51.116.188.112:9092'],
     auto_offset_reset='earliest',
     enable_auto_commit=True,
     group_id=None)

for message in consumer:
    # print('MSG: {}'.format(message.value))
    print(message)
    print('recv')
