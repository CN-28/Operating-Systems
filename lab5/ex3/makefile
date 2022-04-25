CC = gcc -Wall

clean:
	rm -f consumer main producer my_fifo test_files/many*.txt test_files/one*.txt

run:
	make clean
	$(CC) main.c -o main
	$(CC) consumer.c -o consumer
	$(CC) producer.c -o producer

test:
	make clean
	make run
	./main
	make clean