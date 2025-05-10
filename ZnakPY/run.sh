#!/bin/bash

# Компиляция C++ кода
echo "🛠  Компилируем C++ код..."
cd ZnakCPP
chmod +x compile.sh
./compile.sh
cd ..

# Проверяем Python
if ! command -v python3 &> /dev/null
then
    echo "❌ Python3 не установлен. Установите его сначала."
    exit 1
fi

# Создаем виртуальное окружение (если его нет)
if [ ! -d "venv" ]; then
    echo "🐍 Создаем виртуальное окружение..."
    python3 -m venv venv
fi

# Активируем окружение
echo "🔧 Активируем виртуальное окружение..."
source venv/bin/activate

# Устанавливаем зависимости
echo "📦 Устанавливаем зависимости..."
pip install -q aiogram python-dotenv

# Создаем БД если ее нет
echo "🛢  Проверяем базу данных..."
if [ ! -f "data/maindb.sqlite3" ]; then
    echo "🔨 Создаем новую БД..."
    mkdir -p data
    ./app/database/db_worker '{"action":"init_db","db_path":"data/maindb.sqlite3"}' > /dev/null
    
    if [ $? -eq 0 ]; then
        echo "✅ БД успешно создана"
    else
        echo "❌ Ошибка при создании БД"
        exit 1
    fi
else
    echo "ℹ️ БД уже существует"
fi

# Запускаем бота
echo "🤖 Запускаем бота..."
python3 -m app.main

deactivate