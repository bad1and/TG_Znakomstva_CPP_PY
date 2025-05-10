from app.database.db_client import call_cpp

async def get_user(tg_id: int):
    return call_cpp("get_user", {"tg_id": tg_id})

async def add_user(tg_id: int, username: str) -> bool:
    """Добавляет пользователя в БД"""
    response = call_cpp(
        action="add_user",
        data={
            "tg_id": tg_id,
            "username": username
        }
    )
    return response.get("status") == "success"