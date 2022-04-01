CC = gcc -Wall

clean:
	rm -f main
	rm -f w*.txt
run:
	make clean
	$(CC) main.c -lm -o main
	./main 250
	./main 500
	./main 750
	./main 1000