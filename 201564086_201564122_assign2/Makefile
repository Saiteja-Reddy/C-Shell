LDFLAGS=-lncurses

a.out: main.o ls.o echo.o pinfo.o nightswatch.o
	gcc main.o ls.o echo.o pinfo.o nightswatch.o -o a.out -lncurses

main.o: main.c
	gcc -c main.c

ls.o: ls_implement.h ls_implement.c
	gcc -o ls.o -c ls_implement.c

echo.o: echo_implement.h echo_implement.c
	gcc -o echo.o -c echo_implement.c

pinfo.o: pinfo_implement.h pinfo_implement.c
	gcc -o pinfo.o -c pinfo_implement.c

nightswatch.o: nightswatch_implement.h nightswatch_implement.c
	gcc -c nightswatch_implement.c -o nightswatch.o

clean:
	rm -rf main.o ls.o echo.o pinfo.o nightswatch.o a.out
