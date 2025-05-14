import subprocess
import json
import logging
from pathlib import Path
from typing import Dict, Any

logger = logging.getLogger(__name__)


def call_cpp(request: Dict[str, Any]) -> Dict[str, Any]:
    """Call C++ worker with JSON request and return parsed response"""
    cpp_bin = Path(__file__).parent / "db_worker"
    if not cpp_bin.exists():
        print(f"C++ worker not found at {cpp_bin}")
        return {"error": "C++ worker not available"}

    try:
        request_str = json.dumps(request)
        result = subprocess.run(
            [str(cpp_bin), request_str],
            capture_output=True,
            text=True,
            timeout=5  # 5 seconds timeout
        )

        if result.returncode != 0:
            print(f"C++ worker failed with code {result.returncode}: {result.stderr}")
            return {"error": result.stderr or "C++ worker failed"}

        return json.loads(result.stdout)

    except json.JSONDecodeError as e:
        print(f"Failed to parse C++ response: {e}")
        return {"error": "Invalid JSON response"}
    except subprocess.TimeoutExpired:
        print("C++ worker timeout exceeded")
        return {"error": "Timeout exceeded"}
    except Exception as e:
        print(f"Unexpected error calling C++ worker: {e}")
        return {"error": str(e)}
