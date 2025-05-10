from app.database.db_client import call_cpp

async def get_user(tg_id: int):
    return call_cpp("get_user", {"tg_id": tg_id})

