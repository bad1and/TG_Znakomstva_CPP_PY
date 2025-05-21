# app/logger.py
import logging

logger = logging.getLogger("bot_logger")
logger.setLevel(logging.DEBUG)

file_handler = logging.FileHandler("bot.log", encoding="utf-8")
file_handler.setLevel(logging.ERROR)
file_formatter = logging.Formatter("%(asctime)s - [%(levelname)s] - %(name)s - %(message)s")
file_handler.setFormatter(file_formatter)

console_handler = logging.StreamHandler()
console_handler.setLevel(logging.CRITICAL)
console_formatter = logging.Formatter("%(asctime)s - [%(levelname)s] - %(message)s")
console_handler.setFormatter(console_formatter)

logger.addHandler(file_handler)
logger.addHandler(console_handler)

# Чтобы логи из других модулей тоже попадали
logger.propagate = False
