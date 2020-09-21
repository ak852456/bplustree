compile:
    make

execute:
    make run

clean:
    make clean

debug:
    gdb --args ./exec argv1 arvg2 ...

STEPS:
    1. Reset MEMORY_SIZE and DATA_PATH in Makefile
    2. compile and execute

After executing the program, press 1 to do the insertion.
And then input the deletion ration and run loops.
Finaly when the program is finished, press 7 to ouput the outcome.
