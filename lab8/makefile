CC = gcc -Wall -pthread

clean:
	rm -f negative lena_res1.ascii.pgm lena_res2.ascii.pgm ag1.ascii.pgm ag2.ascii.pgm ml1.ascii.pgm ml2.ascii.pgm Times.txt
run:
	make clean
	$(CC) negative.c -o negative

	for number in 1 2 4 8 16 ; do \
		./negative $$number block lena.ascii.pgm lena_res1.ascii.pgm ; \
		./negative $$number numbers lena.ascii.pgm lena_res2.ascii.pgm ; \
	done

	for number in 1 2 4 8 16 ; do \
		./negative $$number block apollonian_gasket.ascii.pgm ag1.ascii.pgm; \
		./negative $$number numbers apollonian_gasket.ascii.pgm ag2.ascii.pgm; \
	done
	
	for number in 1 2 4 8 16 ; do \
		./negative $$number block mona_lisa.ascii.pgm ml1.ascii.pgm ; \
		./negative $$number numbers mona_lisa.ascii.pgm ml2.ascii.pgm ; \
	done