FLAGS = -ansi -Wall -Wextra -Werror -pedantic-errors
LIBS = -lm

all: main.o spmat.o
	gcc main.o spmat.o -o ex2 $(LIBS)

clean:
	rm -rf *.o ex2
	
main.o: main.c spmat.h SPBufferset.h
	gcc $(FLAGS) -c main.c
	
spmat.o: spmat.c SPBufferset.h spmat.h
	gcc $(FLAGS) -c spmat.c
