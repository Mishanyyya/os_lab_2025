# Makefile для сборки программ

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = parallel_min_max process_memory zombie parallel_sum
LIBRARIES = libsumutils.a

# Основная цель - собирает все программы
all: $(TARGETS)

# Статическая библиотека для sum_utils
libsumutils.a: sum_utils.o
	ar rcs $@ $^

sum_utils.o: sum_utils.c sum_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Программа из задания 1 (параллельный min/max с таймаутом)
parallel_min_max: parallel_min_max.c find_min_max.c utils.c
	$(CC) $(CFLAGS) -o $@ $^

# Программа из задания 3 (информация о памяти процесса)
process_memory: process_memory.c
	$(CC) $(CFLAGS) -o $@ $<

# Программа для демонстрации зомби-процессов
zombie: zombie.c
	$(CC) $(CFLAGS) -o $@ $<

# Программа из задания 5 (параллельная сумма с библиотекой)
parallel_sum: parallel_sum.c utils.c libsumutils.a
	$(CC) $(CFLAGS) -pthread -o $@ parallel_sum.c utils.c -L. -lsumutils

# Очистка собранных файлов
clean:
	rm -f $(TARGETS) $(LIBRARIES) *.o

# Пересборка
rebuild: clean all

# Псевдонимы
build: all
zombie-demo: zombie
memory-demo: process_memory
minmax-demo: parallel_min_max
sum-demo: parallel_sum

# Информация о целях
help:
	@echo "Доступные цели:"
	@echo "  all          - собрать все программы"
	@echo "  parallel_min_max - программа с таймаутом (задание 1)"
	@echo "  process_memory   - информация о памяти (задание 3)"
	@echo "  zombie       - демо зомби-процессов"
	@echo "  parallel_sum - параллельная сумма (задание 5)"
	@echo "  libsumutils.a- библиотека для вычисления суммы"
	@echo "  clean        - удалить собранные файлы"
	@echo "  rebuild      - пересобрать всё"

.PHONY: all clean rebuild build help zombie-demo memory-demo minmax-demo sum-demo