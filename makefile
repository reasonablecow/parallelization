CC	=	mpicc
CFLAGS	=	-Wall -Wextra -pedantic -std=c11
LIBS	=	-lm

default:	test-all clean

test-all:	test-sequential test-sequential-o3 test-parallel-task test-parallel-task-o3 test-parallel-data test-parallel-data-o3

test-sequential:	sequential
	./test.sh ./sequential

sequential:	parallel_task.c
	gcc parallel_task.c -o sequential

test-sequential-o3:	sequential_o3
	./test.sh ./sequential_o3

sequential_o3:	parallel_task.c
	gcc parallel_task.c -O3 -o sequential_o3

test-parallel-task:	parallel_task
	./test.sh ./parallel_task

parallel_task:	parallel_task.c
	gcc parallel_task.c -fopenmp -o parallel_task

test-parallel-task-o3:	parallel_task_o3
	./test.sh ./parallel_task_o3

parallel_task_o3:	parallel_task.c
	gcc parallel_task.c -fopenmp -O3 -o parallel_task_o3

test-parallel-data:	parallel_data
	./test.sh ./parallel_data

parallel_data:	parallel_data.c
	gcc parallel_data.c -fopenmp -o parallel_data

test-parallel-data-o3:	parallel_data_o3
	./test.sh ./parallel_data_o3

parallel_data_o3:	parallel_data.c
	gcc parallel_data.c -fopenmp -O3 -o parallel_data_o3

test-multiprocessing:	multiprocessing
	./test.sh "mpirun -np 4 multiprocessing"

multiprocessing:	multiprocessing.c
	$(CC) $(CFLAGS) -o multiprocessing multiprocessing.c $(LIBS)

clean:
	rm -f \
	   sequential \
	   sequential_o3 \
	   parallel_task \
	   parallel_task_o3 \
	   parallel_data \
	   parallel_data_o3 \
	   multiprocessing \
	   ;
