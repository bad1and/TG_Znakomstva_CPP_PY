import os
from pathlib import Path
from dotenv import load_dotenv
from  app.logger import logger

# Загрузка .env из корня проекта
env_path = Path(__file__).parent.parent / '.env'
load_dotenv(env_path)


class Config:
    TOKEN = os.getenv('TOKEN')
    ADMIN_ID = int(os.getenv('ADMIN_ID', 0))
    DB_PATH = os.path.join(Path(__file__).parent.parent, 'data/maindb.sqlite3')

if not Config.TOKEN:
    logger.critical("Отсутствует TOKEN в .ENV")
else:
    logger.critical("TOKEN - OK")

if not Config.ADMIN_ID:
    logger.critical("Отсутствует ADMIN_ID в .ENV")
else:
    logger.critical("ADMIN_ID - OK")