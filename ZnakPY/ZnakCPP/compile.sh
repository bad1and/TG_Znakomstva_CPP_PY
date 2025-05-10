#!/bin/bash

# Компиляция с оптимизацией
g++ -std=c++11 db_worker.cpp \
    -lsqlite3 \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -O2 \
    -o ../app/database/db_worker

# Проверка успешности компиляции
if [ -f "../app/database/db_worker" ]; then
    chmod +x ../app/database/db_worker
    echo "✅ db_worker успешно обновлён"
else
    echo "❌ Ошибка компиляции!"
    exit 1
fi