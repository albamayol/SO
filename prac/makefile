all: Bowman.exe Poole.exe Discovery.exe

Bowman.exe: Bowman.o Global.o Trama.o
	gcc Bowman.o Global.o Trama.o -o Bowman.exe -pthread -Wall -Wextra -ggdb

Poole.exe: Poole.o Global.o Trama.o
	gcc Poole.o Global.o Trama.o -o Poole.exe -pthread -Wall -Wextra -ggdb

Discovery.exe: Discovery.o Global.o Trama.o
	gcc Discovery.o Global.o Trama.o -o Discovery.exe -pthread -Wall -Wextra -ggdb

Global.o: Global.c Global.h 
	gcc -c Global.c -pthread -Wall -Wextra -ggdb

Trama.o: Trama.c Trama.h Global.h
	gcc -c Trama.c -pthread -Wall -Wextra -ggdb

Bowman.o: Bowman.c Global.h Trama.h
	gcc -c Bowman.c -pthread -Wall -Wextra -ggdb

Poole.o: Poole.c Global.h Trama.h
	gcc -c Poole.c -pthread -Wall -Wextra -ggdb

Discovery.o: Discovery.c Global.h Trama.h
	gcc -c Discovery.c -pthread -Wall -Wextra -ggdb

.PHONY: clean
clean:
	rm *.o
	rm Bowman.exe
	rm Poole.exe
	rm Discovery.exe