OBJS = main.o functions.o wordList.o server-client.o
SRC = main.c functions.c wordList.c server-client.c
HEAD = hash.h oracle.h functions.h wordList.h server-client.h global.h
OUT = invoke-oracle-multithreaded
compiler = gcc
flags = -g -c
executable = invoke-oracle-multithreaded

all: $(OBJS)
	$(compiler) -g $(OBJS) -lpthread -L. -loracle_v3 -lhash -o $(OUT)

main.o: main.c functions.h
	$(compiler) $(flags) main.c

functions.o: functions.c functions.h wordList.h global.h
	$(compiler) $(flags) functions.c

wordList.o: wordList.c wordList.h
	$(compiler) $(flags) wordList.c

server-client.o: server-client.c server-client.h
	$(compiler) $(flags) server-client.c

clean:
	rm $(executable) $(OBJS)
count:
	wc $(SRC) $(HEAD)
