import os
import subprocess

# Абсолютный путь к корню проекта
project_root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))

# Путь к исполняемому файлу create_db
cpp_executable = os.path.join(project_root, 'create_db')

# Запуск C++ кода
try:
    result = subprocess.run([cpp_executable], check=True)
    print("C++: база данных создана.")
except Exception as e:
    print(f"Ошибка при запуске C++: {e}")
