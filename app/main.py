from aiogram import Bot, Dispatcher
from app.config import Config
from app.handlers import router
import logging


async def main():
    bot = Bot(token=Config.TOKEN)
    dp = Dispatcher()
    dp.include_router(router)
    await dp.start_polling(bot)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    import asyncio

    asyncio.run(main())
