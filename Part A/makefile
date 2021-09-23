.DEFAULT_GOAL := lift_sim_A
CC = gcc
CFLAGS = -Wall -ansi -pedantic -Werror -std=c99 -Iheaders -Iobjects -g
OBJ = objects/file_io.o objects/lift_sim_A.o objects/thread.o
LIBS = -lpthread
EXEC = lift_sim_A
HEADERS = headers/Request.h headers/file_io.h headers/lift_sim_A.h headers/shared_var.h

objects/thread.o: headers/thread.h headers/shared_var.h
	$(CC) -c src/thread.c $(CFLAGS)
	mv thread.o objects

objects/file_io.o: headers/file_io.h headers/Request.h headers/shared_var.h
	$(CC) -c src/file_io.c $(CFLAGS)
	mv file_io.o objects

objects/lift_sim_A.o: headers/file_io.h headers/Request.h headers/lift_sim_A.h headers/thread.h
	$(CC) -c src/lift_sim_A.c $(CFLAGS)
	mv lift_sim_A.o objects

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)

clean:
	rm $(EXEC)
	rm -rf objects/*.o
	rm sim_out
	rm sim_input
