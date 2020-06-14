OPTS=-Wall -g -std=c++11
COMOBJS=dates.o parse-todo.o listing.o
OBJSPTDI=$(COMOBJS) ptodo.o filterDPCP.o
CC=g++

all: ptodo


ptodo: $(OBJSPTDI)
	$(CC) $(OPTS) $(OBJSPTDI) -o ptodo

ptodo.o: ptodo.cc dates.h parse-todo.h filterDPCP.h listing.h
	$(CC) $(OPTS) -c ptodo.cc

filterDPCP.o: filterDPCP.cc filterDPCP.h
	$(CC) $(OPTS) -c filterDPCP.cc


dates.o: dates.cc dates.h
	$(CC) $(OPTS) -c dates.cc

listing.o: listing.cc listing.h
	$(CC) $(OPTS) -c listing.cc

parse-todo.o: parse-todo.cc parse-todo.h
	$(CC) $(OPTS) -c parse-todo.cc

clean:
	rm -f $(OBJSPTDI)
