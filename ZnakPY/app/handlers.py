from aiogram import Router, F
from aiogram.filters import CommandStart
from aiogram.types import Message
from app.database.requests import add_user
from app.keyboards import menu

router = Router()

@router.message(CommandStart())
async def cmd_start(message: Message):
    await message.answer(
        f"Привет, {message.from_user.first_name}!",
        reply_markup=menu)
    try:
        # Получаем данные из сообщения
        user = message.from_user
        success = await add_user(
            tg_id=user.id,
            username=user.username or "unknown"  # На случай, если username не указан
        )

        if success:
            await message.answer("✅ Ваши данные сохранены!")
        else:
            await message.answer("❌ Ошибка сохранения данных")

    except Exception as e:
        await message.answer("⚠️ Произошла техническая ошибка")

