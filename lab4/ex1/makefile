CC = gcc -Wall

clean:
	rm -f test_exec fork exec
run:
	$(CC) fork.c -o fork
	$(CC) test_exec.c -o test_exec
	$(CC) exec.c -o exec
	@echo ""
	./fork ignore
	@echo ""
	./fork handler
	@echo ""
	./fork mask
	@echo ""
	./fork pending
	@echo ""
	./exec ignore
	@echo ""
	./exec mask
	@echo ""
	./exec pending
	@echo ""
	make clean