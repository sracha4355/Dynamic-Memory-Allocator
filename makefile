
main: alloc.o main.o
	@echo "linking and creating executable"
	gcc alloc.o main.o -o main

alloc.o:
	@echo "creating object file for alloc.c"
	gcc alloc.c -c
main.o:
	@echo "creating object file for main.c"
	gcc main.c -c

clean:
	@echo "cleaning artifacts"
	@rm *.o
	@rm main
rebuild:
	@echo "rebuilding"
	@make clean
	@make main
