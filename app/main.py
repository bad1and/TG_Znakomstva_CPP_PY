# main.py
from aiogram import Bot, Dispatcher
from app.config import Config
from app.handlers import router
from app.logger import logger
import asyncio


async def main():
    bot = Bot(token=Config.TOKEN)
    dp = Dispatcher()
    dp.include_router(router)

    logger.critical("Бот запущен...")
    await dp.start_polling(bot)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.critical("Бот остановлен пользователем")
    except Exception as e:
        logger.critical("Фатальная ошибка в боте: %s", e, exc_info=True)
