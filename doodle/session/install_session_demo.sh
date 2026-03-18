#!/usr/bin/env bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cp "${ROOT_DIR}/doodle/session/session_test.html" "${ROOT_DIR}/resources/web/session_test.html"
cp "${ROOT_DIR}/doodle/session/session_echo.py" "${ROOT_DIR}/resources/web/session_echo.py"
chmod +x "${ROOT_DIR}/resources/web/session_echo.py"
printf 'Installed session demo files into resources/web/.\n'
