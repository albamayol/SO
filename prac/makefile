all: Bowman.exe Poole.exe Discovery.exe

Bowman.exe: Bowman.o Global.o Trama.o Strings.o Commands.o FilesCreation.o Logica.o Screen.o
	gcc Bowman.o Global.o Trama.o Strings.o Commands.o FilesCreation.o Logica.o Screen.o -o Bowman.exe -pthread -Wall -Wextra -ggdb

Poole.exe: Poole.o Global.o Trama.o Strings.o Commands.o FilesCreation.o Logica.o Screen.o
	gcc Poole.o Global.o Trama.o Strings.o Commands.o FilesCreation.o Logica.o Screen.o -o Poole.exe -pthread -Wall -Wextra -ggdb

Discovery.exe: Discovery.o Global.o Trama.o Strings.o Commands.o FilesCreation.o Logica.o Screen.o
	gcc Discovery.o Global.o Trama.o Strings.o Commands.o FilesCreation.o Logica.o Screen.o -o Discovery.exe -pthread -Wall -Wextra -ggdb

Strings.o: Strings.c Strings.h
	gcc -c Strings.c -pthread -Wall -Wextra -ggdb

Commands.o: Commands.c Commands.h
	gcc -c Commands.c -pthread -Wall -Wextra -ggdb

FilesCreation.o: FilesCreation.c FilesCreation.h
	gcc -c FilesCreation.c -pthread -Wall -Wextra -ggdb

Logica.o: Logica.c Logica.h
	gcc -c Logica.c -pthread -Wall -Wextra -ggdb

ConfigurationBowman.o: ConfigurationBowman.c ConfigurationBowman.h
	gcc -c ConfigurationBowman.c -pthread -Wall -Wextra -ggdb

ConfigurationPoole.o: ConfigurationPoole.c ConfigurationPoole.h
	gcc -c ConfigurationPoole.c -pthread -Wall -Wextra -ggdb

ConfigurationDiscovery.o: ConfigurationDiscovery.c ConfigurationDiscovery.h
	gcc -c ConfigurationDiscovery.c -pthread -Wall -Wextra -ggdb

Screen.o: Screen.c Screen.h
	gcc -c Screen.c -Wall -Wextra -ggdb

Global.o: Global.c Global.h Strings.h Commands.h FilesCreation.h Logica.h Screen.h
	gcc -c Global.c -pthread -Wall -Wextra -ggdb

Trama.o: Trama.c Trama.h Global.h Strings.h Commands.h FilesCreation.h Logica.h Screen.h
	gcc -c Trama.c -pthread -Wall -Wextra -ggdb

Bowman.o: Bowman.c Global.h Trama.h Strings.h Commands.h FilesCreation.h Logica.h Screen.h
	gcc -c Bowman.c -pthread -Wall -Wextra -ggdb

Poole.o: Poole.c Global.h Trama.h Strings.h Commands.h FilesCreation.h Logica.h Screen.h
	gcc -c Poole.c -pthread -Wall -Wextra -ggdb

Discovery.o: Discovery.c Global.h Trama.h Strings.h Commands.h FilesCreation.h Logica.h Screen.h
	gcc -c Discovery.c -pthread -Wall -Wextra -ggdb

.PHONY: clean
clean:
	rm *.o
	rm Bowman.exe
	rm Poole.exe
	rm Discovery.exe