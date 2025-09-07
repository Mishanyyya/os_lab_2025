#!/bin/bash

count=0
sum=0

for arg in "$@"; do
    sum=$(awk "BEGIN {print $sum + $arg}")
    count=$((count + 1))
done

if [ "$count" -gt 0 ]; then
  average=$(awk "BEGIN {print $sum / $count}")
  echo "Количество аргументов: $count"
  echo "Среднее арифметическое: $average"
else
  echo "Не найдено ни одного корректного числового аргумента."
fi
