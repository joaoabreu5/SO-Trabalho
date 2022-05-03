all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o obj/parser.o obj/readln.o 
	gcc -Wall -g obj/sdstored.o obj/parser.o obj/readln.o -o bin/sdstored

obj/sdstored.o: src/sdstored.c src/declarations.h
	gcc -Wall -g -c src/sdstored.c -o obj/sdstored.o
	
bin/sdstore: obj/sdstore.o 
	gcc -Wall -g obj/sdstore.o -o bin/sdstore
	
obj/sdstore.o: src/sdstore.c src/declarations.h
	gcc -Wall -g -c src/sdstore.c -o obj/sdstore.o

obj/parser.o: src/parser.c src/parser.h
	gcc -Wall -g -c src/parser.c -o obj/parser.o

obj/readln.o: src/readln.c src/readln.h
	gcc -Wall -g -c src/readln.c -o obj/readln.o

obj/declarations.o : src/declarations.h
	gcc -Wall -g -c src/declarations.h -o obj/declarations.o
	
clean:
	rm obj/*.o bin/sdstore bin/sdstored