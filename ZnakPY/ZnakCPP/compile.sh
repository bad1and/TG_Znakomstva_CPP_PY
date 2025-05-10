#!/bin/bash

g++ -std=c++11 db_worker.cpp \
    -lsqlite3 \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -o db_worker

if [ -f db_worker ]; then
    chmod +x db_worker
    mkdir -p ../app/database/
    mv db_worker ../app/database/
    echo "✅ Компиляция успешна!"
else
    echo "❌ Ошибка компиляции"
fi