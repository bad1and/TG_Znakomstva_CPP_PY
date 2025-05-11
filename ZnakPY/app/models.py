from aiogram.fsm.state import StatesGroup, State
from sqlalchemy.ext.asyncio import async_sessionmaker, AsyncSession
from typing import AsyncGenerator

# Состояния FSM (оставляем как есть)
class RegistrationState(StatesGroup):
    waiting_for_sex = State()
    waiting_for_opros = State()
    waiting_for_bot_name = State()
    waiting_for_age = State()
    waiting_for_name = State()

class Avatarka(StatesGroup):
    waiting_for_pic = State()

# Асинхронная сессия (упрощенная версия)
async_session = async_sessionmaker(
    expire_on_commit=False,
    class_=AsyncSession,
    autoflush=False
)

async def get_async_session() -> AsyncGenerator[AsyncSession, None]:
    async with async_session() as session:
        yield session