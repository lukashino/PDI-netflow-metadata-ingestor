from kafka import KafkaConsumer
from json import loads
import sys,getopt

broker = ['51.116.188.112:9092']
topic = 'pdi'
full_cmd_arguments = sys.argv
argument_list = full_cmd_arguments[1:]
short_options = "b:t:h"

try:
    arguments, values = getopt.getopt(argument_list, short_options, "")
except getopt.error as err:
    print (str(err))
    sys.exit(2)
for current_argument, current_value in arguments:
    if current_argument in ("-b"):
        broker[0] = current_value
    elif current_argument in ("-h"):
        print ("    Usage:\n     -b <broker>   ~~~~ sets broker (default 51.116.188.112:9092)\n     -t <topic>    ~~~~ sets topic  (default pdi)")
        sys.exit(0)
    elif current_argument in ("-t"):
        topic = current_value

consumer = KafkaConsumer(
	topic,
	bootstrap_servers=broker,
	auto_offset_reset='latest',
	enable_auto_commit=True,
	group_id=None)

for message in consumer:
	# print(message)
	print("-------------------------------------------------------")
	print(message.value.decode("utf-8"))
