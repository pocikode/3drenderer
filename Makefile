build:
	gcc -Wall -std=c23 ./src/*.c -lSDL2 -o renderer

run:
	./renderer

clean:
	rm renderer
