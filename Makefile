all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o obj/parser.o obj/readln.o 
	gcc -g obj/sdstored.o obj/parser.o obj/readln.o -o bin/sdstored

obj/sdstored.o: src/sdstored.c
	gcc -Wall -g -c src/sdstored.c -o obj/sdstored.o
	
bin/sdstore: obj/sdstore.o
	gcc -g obj/sdstore.o -o bin/sdstore
	
obj/sdstore.o: src/sdstore.c
	gcc -Wall -g -c src/sdstore.c -o obj/sdstore.o

obj/parser.o: src/parser.c
	gcc -Wall -g -c src/parser.c -o obj/parser.o

obj/readln.o: src/readln.c
	gcc -Wall -g -c src/readln.c -o obj/readln.o
	
clean:
	rm obj/* tmp/* bin/*