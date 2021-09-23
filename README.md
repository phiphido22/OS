# OS
C Programming Project focused on learning how to implement threads and mutex locks.

Brief:  This project creates a simulation of a multiprocessor system. The way
        that this is implemented is through the use of the p_thread.h library.
        Threads are made for each CPU as well as a thread to add tasks for the
        CPUs to execute. The program exits when all tasks from the designated
        task file is complete.

Compiling:  This Project contains a make file. The program can be compiled
            by using:

            make Command: "make"

            To remove all the object files, stored in the objects folder, the
            executable and all other temporary files use:
            **NOTE** THIS REMOVES THE LOG FILE 

            make Command: "make clean"

Operation:  Once the Project is compiled. You can utilize the program's
            two different settings you will have to run the program with the
            appropriate command.

            Part A: Specifying both the task file and the Ready Queue Size
            Command: ./lift_sim_A m t
            Where:
                    m = size of the buffer
                    t = wait time of the lift operation

	    Part B: Specifying both the task file and the Ready Queue Size
            Command: ./lift_sim_B m t
            Where:
                    m = size of the buffer
                    t = wait time of the lift operation

Files:  This project is split into 2 main directories, Part A and Part B

        Assignment:
            - declaration_of_originality.pdf
            - Assignment Report.pdf
            - read_me.txt

        Assignment/Part A:
	    - makefile

	Assignment/Part A/headers:
	    - file_io.h
	    - lift_sim_a.h
	    - Request.h
	    - shared_var.h
	    - thread.h

	Assignment/Part A/src:
	    - file_io.c
	    - lift_sim_A.c
	    - thread.c

        Assignment/Part B:
            - lift_sim_B.h
	    - lift_sim_B.c
	    - makefile
   	    - Request.h


Testing: This program has been tested on
    Windows Ubuntu Subsystem: Ubuntu 18.04.1 LTS
    Fedora Linux: Fedora 5.0.6 Workstation Edition
