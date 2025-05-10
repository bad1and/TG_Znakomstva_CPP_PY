from aiogram import Router, F
from aiogram.filters import CommandStart
from aiogram.types import Message

from app.keyboards import menu

router = Router()


@router.message(CommandStart())
async def cmd_start(message: Message):
    await message.answer("Робит!")