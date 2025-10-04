CC=gcc
CFLAGS=-I.

# собирает всё
all: sequential_min_max parallel_min_max run_seq

# сборка последовательной версии
sequential_min_max : utils.o find_min_max.o sequential_min_max.c utils.h find_min_max.h
	$(CC) -o sequential_min_max find_min_max.o utils.o sequential_min_max.c $(CFLAGS)

# сборка параллельной версии
parallel_min_max : utils.o find_min_max.o parallel_min_max.c utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

# для программы-запускателя (задание 4)
run_seq : run_seq.c
	$(CC) -o run_seq run_seq.c $(CFLAGS)

# объектники
utils.o : utils.c utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : find_min_max.c find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

# очистка
clean :
	rm -f utils.o find_min_max.o sequential_min_max parallel_min_max run_seq
