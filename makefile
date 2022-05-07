FLAGS = -Wall -Wextra -Werror -pedantic -std=c11
LIBS = -lm
TARGETS = sequential \
          sequential_o3 \
          parallel_task \
          parallel_task_o3 \
          parallel_data \
          parallel_data_o3 \
          multiprocessing \
          multiprocessing_o3
TESTS = $(addprefix test_,$(TARGETS))

.PHONY: default test $(TESTS) $(TARGETS) clean

default: $(TARGETS)


test: $(TESTS) clean

test_sequential \
test_sequential_o3: test_%: build/%
	./test.sh $^
test_parallel_task \
test_parallel_task_o3 \
test_parallel_data \
test_parallel_data_o3: test_%: build/%
	OMP_NUM_THREADS=8 ./test.sh $^
test_multiprocessing \
test_multiprocessing_o3: test_%: build/%
	OMP_NUM_THREADS=2 ./test.sh "mpirun -np 4 $^"


$(TARGETS): %: build/%

build/sequential: src/parallel_task.c build
	gcc $(FLAGS) -Wno-unknown-pragmas -o $@ $<
build/sequential_o3: src/parallel_task.c build
	gcc $(FLAGS) -Wno-unknown-pragmas -O3 -o $@ $<
build/parallel_task \
build/parallel_data: build/%: src/%.c build
	gcc $(FLAGS) -fopenmp -o $@ $<
build/parallel_task_o3 \
build/parallel_data_o3: build/%_o3: src/%.c build
	gcc $(FLAGS) -fopenmp -O3 -o $@ $<
build/multiprocessing: src/multiprocessing.c build
	mpicc $(FLAGS) -fopenmp -o $@ $<
build/multiprocessing_o3: src/multiprocessing.c build
	mpicc $(FLAGS) -fopenmp -O3 -o $@ $<

build:
	mkdir -p $@

clean:
	rm -rfv build
