all:
	gcc -c -Wall -Wformat src/treatment.c
	mv treatment.o src/
	gcc -Wall -Wformat -o testaro src/treatment.o src/testaro.c

clean:
	rm testaro