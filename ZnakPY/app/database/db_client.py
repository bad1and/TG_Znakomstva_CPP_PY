import subprocess
import json
from pathlib import Path
from app.config import Config


def call_cpp(action: str, data: dict) -> dict:
    cpp_path = Path(__file__).parent / "db_worker"
    request = json.dumps({
        "action": action,
        "db_path": str(Config.DB_PATH),  # Путь из конфига
        **data
    })

    try:
        result = subprocess.run(
            [str(cpp_path), request],
            capture_output=True,
            text=True,
            check=True
        )
        return json.loads(result.stdout)
    except Exception as e:
        return {"error": str(e)}