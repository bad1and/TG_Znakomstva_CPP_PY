from aiogram import Router, F
from aiogram.filters import CommandStart
from aiogram.types import Message
from app.database.requests import add_user
from app.keyboards import menu

router = Router()


@router.message(CommandStart())
async def cmd_start(message: Message):
    result = await add_user(message.from_user.id, message.from_user.username or "unknown")

    if result == "created":
        await message.answer("✅ Вы зарегистрированы!")
    elif result == "exists":
        await message.answer("ℹ️ Вы уже зарегистрированы")
    else:
        await message.answer("❌ Ошибка регистрации")