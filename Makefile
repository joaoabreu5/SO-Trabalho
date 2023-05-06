all: directories server client

server: directories bin/sdstored

client: directories bin/sdstore

bin/sdstored: obj/sdstored.o obj/parser.o obj/readln.o obj/queue.o obj/declarations.o
	gcc -Wall -g obj/sdstored.o obj/parser.o obj/readln.o obj/queue.o obj/declarations.o -o bin/sdstored

obj/sdstored.o: src/sdstored.c
	gcc -Wall -g -c src/sdstored.c -o obj/sdstored.o
	
bin/sdstore: obj/sdstore.o  obj/declarations.o
	gcc -Wall -g obj/sdstore.o obj/declarations.o -o bin/sdstore
	
obj/sdstore.o: src/sdstore.c
	gcc -Wall -g -c src/sdstore.c -o obj/sdstore.o

obj/parser.o: src/parser.c src/parser.h
	gcc -Wall -g -c src/parser.c -o obj/parser.o

obj/readln.o: src/readln.c src/readln.h
	gcc -Wall -g -c src/readln.c -o obj/readln.o

obj/queue.o: src/queue.c src/queue.h
	gcc -Wall -g -c src/queue.c -o obj/queue.o

obj/declarations.o: src/declarations.c src/declarations.h
	gcc -Wall -g -c src/declarations.c -o obj/declarations.o

directories: 
	mkdir -p bin obj

clean:
	rm -f obj/*.o bin/sdstore bin/sdstored
	rmdir obj bin
