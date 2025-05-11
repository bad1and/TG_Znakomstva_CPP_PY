import logging
from typing import Optional, Dict, Any
from pathlib import Path
from app.database.db_client import call_cpp
from app.config import Config

logger = logging.getLogger(__name__)


def _validate_response(response: Dict[str, Any], required_fields: list = None) -> bool:
    if not response:
        logger.error("Empty response from C++ worker")
        return False

    if "error" in response:
        logger.error(f"C++ worker error: {response['error']}")
        return False

    if required_fields:
        for field in required_fields:
            if field not in response:
                logger.error(f"Missing required field in response: {field}")
                return False

    return True


async def add_user(
        tg_id: int,
        username: str,
        first_name: Optional[str] = None,
        last_name: Optional[str] = None,
        number: Optional[str] = None,
        in_bot_name: Optional[str] = None,
        sex: Optional[str] = None,
        years: Optional[int] = None,
        unic_your_id: Optional[str] = None,
        unic_wanted_id: Optional[str] = None,
        status: int = 1
) -> bool:
    """Add new user or update existing one"""
    request = {
        "action": "add_user",
        "tg_id": tg_id,
        "tg_username": username,
        "first_name": first_name,
        "last_name": last_name,
        "number": number,
        "in_bot_name": in_bot_name,
        "sex": sex,
        "years": years,
        "unic_your_id": unic_your_id,
        "unic_wanted_id": unic_wanted_id,
        "status": status,
        "db_path": str(Config.DB_PATH)
    }

    response = call_cpp(request)
    print(f"Add user response: {response}")

    if not _validate_response(response, ["status"]):
        return False

    return response["status"] in ("created", "exists")


async def get_user(tg_id: int) -> Optional[Dict[str, Any]]:
    """Get user by telegram ID"""
    response = call_cpp({
        "action": "get_user",
        "tg_id": tg_id,
        "db_path": str(Config.DB_PATH)
    })

    if not _validate_response(response):
        return None

    return response




async def update_profile(
        tg_id: int,
        in_bot_name: Optional[str] = None,
        sex: Optional[str] = None,
        years: Optional[int] = None
) -> bool:
    """Update user profile information"""
    request = {
        "action": "update_profile",
        "tg_id": tg_id,
        "db_path": str(Config.DB_PATH)
    }

    if in_bot_name is not None:
        request["in_bot_name"] = in_bot_name
    if sex is not None:
        request["sex"] = sex
    if years is not None:
        request["years"] = years

    response = call_cpp(request)
    print(f"Update user response: {response}")

    if not _validate_response(response, ["status"]):
        return False

    return response["status"] == "success"



async def update_questionnaire(
        tg_id: int,
        unic_your_id: Optional[str] = None,
        unic_wanted_id: Optional[str] = None
) -> bool:
    """Update user questionnaire data"""
    request = {
        "action": "update_questionnaire",
        "tg_id": tg_id,
        "db_path": str(Config.DB_PATH)
    }

    if unic_your_id is not None:
        request["unic_your_id"] = unic_your_id
    if unic_wanted_id is not None:
        request["unic_wanted_id"] = unic_wanted_id

    # print(f"REQUEST TO UNIC: {request}")
    response = call_cpp(request)
    print(f"Update_2 user response: {response}")

    if not _validate_response(response, ["status"]):
        return False

    return response["status"] == "success"



async def update_user_status(tg_id: int, status: int) -> bool:
    """Update user active status"""
    response = call_cpp({
        "action": "update_status",
        "tg_id": tg_id,
        "status": status,
        "db_path": str(Config.DB_PATH)
    })

    if not _validate_response(response, ["status"]):
        return False

    return response["status"] == "success"




async def get_users_count() -> int:
    """Get total count of users"""
    response = call_cpp({
        "action": "get_users_count",
        "db_path": str(Config.DB_PATH)
    })

    if not _validate_response(response, ["count"]):
        return 0

    return response["count"]