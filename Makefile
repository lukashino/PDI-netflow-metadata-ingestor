BIN=prod
LIB=linked_list

all:
	gcc -Wall -Wextra $(BIN).c $(LIB).c -o $(BIN) -lrdkafka -lm

clean:
	rm -rf *.o $(BIN)