all: Bowman.exe Poole.exe Discovery.exe

Bowman.exe: Bowman.o Global.o linkedlist.o Trama.o
	gcc Bowman.o Global.o linkedlist.o Trama.o -o Bowman.exe -pthread -Wall -Wextra -g

Poole.exe: Poole.o Global.o linkedlist.o Trama.o
	gcc Poole.o Global.o linkedlist.o Trama.o -o Poole.exe -pthread -Wall -Wextra -g

Discovery.exe: Discovery.o Global.o linkedlist.o Trama.o
	gcc Discovery.o Global.o linkedlist.o Trama.o -o Discovery.exe -pthread -Wall -Wextra -g

linkedlist.o: linkedlist.c linkedlist.h
	gcc -c linkedlist.c -pthread -Wall -Wextra -g

Global.o: Global.c Global.h linkedlist.h
	gcc -c Global.c -pthread -Wall -Wextra -g

Trama.o: Trama.c Trama.h Global.h
	gcc -c Trama.c -pthread -Wall -Wextra -g

Bowman.o: Bowman.c Global.h linkedlist.h Trama.h
	gcc -c Bowman.c -pthread -Wall -Wextra -g

Poole.o: Poole.c Global.h linkedlist.h Trama.h
	gcc -c Poole.c -pthread -Wall -Wextra -g

Discovery.o: Discovery.c Global.h linkedlist.h Trama.h
	gcc -c Discovery.c -pthread -Wall -Wextra -g

.PHONY: clean
clean:
	rm *.o
	rm Bowman.exe
	rm Poole.exe
	rm Discovery.exe