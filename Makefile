BIN=prod

all:
	gcc -Wall -Wextra $(BIN).c -o $(BIN) -lrdkafka -lm

clean:
	rm -rf *.o $(BIN)