FLAGS		=	-Wall -Wextra -Werror -pedantic -std=c11
FLAGS_NOOMP	=	$(FLAGS) -Wno-unknown-pragmas
FLAGS_NOOMP_O3	=	$(FLAGS_NOOMP) -O3
FLAGS_OMP	=	$(FLAGS) -fopenmp
FLAGS_OMP_O3	=	$(FLAGS_OMP) -O3
LIBS		=	-lm
TARGETS		=	sequential \
			sequential_o3 \
			parallel_task \
			parallel_task_o3 \
			parallel_data \
			parallel_data_o3 \
			multiprocessing \
			multiprocessing_o3

default:	$(TARGETS)

clean:
	rm -f $(TARGETS)

test_all:	test_sequential \
		test_sequential_o3 \
		test_parallel_task \
		test_parallel_task_o3 \
		test_parallel_data \
		test_parallel_data_o3 \
		test_multiprocessing \
		test_multiprocessing_o3 \
		clean

test_sequential:		sequential
	./test.sh ./sequential
test_sequential_o3:		sequential_o3
	./test.sh ./sequential_o3
test_parallel_task:		parallel_task
	OMP_NUM_THREADS=8 ./test.sh ./parallel_task
test_parallel_task_o3:		parallel_task_o3
	OMP_NUM_THREADS=8 ./test.sh ./parallel_task_o3
test_parallel_data:		parallel_data
	OMP_NUM_THREADS=8 ./test.sh ./parallel_data
test_parallel_data_o3:		parallel_data_o3
	OMP_NUM_THREADS=8 ./test.sh ./parallel_data_o3
test_multiprocessing:		multiprocessing
	OMP_NUM_THREADS=2 ./test.sh "mpirun -np 4 multiprocessing"
test_multiprocessing_o3:	multiprocessing_o3
	OMP_NUM_THREADS=2 ./test.sh "mpirun -np 4 multiprocessing_o3"

sequential:		parallel_task.c
	gcc	$(FLAGS_NOOMP) -o sequential		parallel_task.c
sequential_o3:		parallel_task.c
	gcc	$(FLAGS_NOOMP_O3) -o sequential_o3	parallel_task.c
parallel_task:		parallel_task.c
	gcc	$(FLAGS_OMP) -o parallel_task		parallel_task.c
parallel_task_o3:	parallel_task.c
	gcc	$(FLAGS_OMP_O3) -o parallel_task_o3	parallel_task.c
parallel_data:		parallel_data.c
	gcc	$(FLAGS_OMP) -o parallel_data		parallel_data.c
parallel_data_o3:	parallel_data.c
	gcc	$(FLAGS_OMP_O3) -o parallel_data_o3	parallel_data.c
multiprocessing:	multiprocessing.c
	mpicc	$(FLAGS_OMP) -o multiprocessing		multiprocessing.c
multiprocessing_o3:	multiprocessing.c
	mpicc	$(FLAGS_OMP_O3) -o multiprocessing_o3	multiprocessing.c
