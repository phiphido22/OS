.DEFAULT_GOAL := lift_sim_B
CC = gcc
CFLAGS = -Wall -ansi -pedantic -Werror -std=c99 -g -D_POSIX_C_SOURCE=200809L -std=gnu11
OBJ = lift_sim_B.o
LIBS = -lpthread -lrt
EXEC = lift_sim_B
HEADERS = Request.h lift_sim_B.h

lift_sim_B.o: $(HEADERS)
	$(CC) -c lift_sim_B.c $(CFLAGS)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)

clean:
	rm $(EXEC)
	rm -rf *.o
	rm sim_input
	rm sim_out
