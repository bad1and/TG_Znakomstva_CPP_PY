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

# Запускаем бота
echo "🤖 Запускаем бота..."
python3 -m app.main

deactivate