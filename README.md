# PDI-netflow-metadata-ingetstor

Before compiling install librdkafka lib
```
apt install librdkafka-dev
```

Compile C
```c
gcc prod.c -o prod -lrdkafka -lm
```

For Python you need `pip3 install kafka-python`.

## Test environment
- run the consumer `python3 cons.py`
- run the prod.c file `./prod`
- message should appear on consumer's window

## Run script
Dependencies:
```
sudo apt-get install -y tcpreplay
```
