from app.logger import logger
import subprocess
import json
from pathlib import Path
from typing import Dict, Any


def call_cpp(request: Dict[str, Any]) -> Dict[str, Any]:
    cpp_bin = Path(__file__).parent / "db_worker"
    if not cpp_bin.exists():
        logger.critical(f"C++ worker not found at {cpp_bin}")
        return {"error": "C++ worker not available"}

    try:
        request_str = json.dumps(request)
        result = subprocess.run(
            [str(cpp_bin), request_str],
            capture_output=True,
            text=True,
            timeout=5
        )

        # logger.debug(f"DB_CLIENT result.subprocess: {result}")

        # print(f"DB_CLIENT result.subprocess: {result}")


        if result.returncode != 0:
            logger.critical(f"C++ worker failed with code {result.returncode}: {result.stderr}")
            return {"error": result.stderr or "C++ worker failed"}

        return json.loads(result.stdout)

    except json.JSONDecodeError as e:
        logger.error(f"Failed to parse C++ response: {e}")
        return {"error": "Invalid JSON response"}
    except subprocess.TimeoutExpired:
        logger.error("C++ worker timeout exceeded")
        return {"error": "Timeout exceeded"}
    except Exception as e:
        logger.error(f"Unexpected error calling C++ worker: {e}", exc_info=True)
        return {"error": str(e)}
