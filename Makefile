BIN=sniffo
LIB=linked_list

all:
	gcc -Wall -Wextra $(BIN).c $(LIB).c -o $(BIN) -lrdkafka -lm -pthread

clean:
	rm -rf *.o $(BIN)