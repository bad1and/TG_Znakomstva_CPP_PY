from app.database.db_client import call_cpp

async def get_user(tg_id: int):
    return call_cpp("get_user", {"tg_id": tg_id})

async def add_user(tg_id: int, username: str) -> str:
    """Возвращает: 'created', 'exists' или 'error'"""
    response = call_cpp("add_user", {
        "tg_id": tg_id,
        "username": username
    })
    return response.get("action", "error")