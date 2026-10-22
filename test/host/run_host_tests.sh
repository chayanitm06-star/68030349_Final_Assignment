#!/usr/bin/env bash
#
# Builds and runs the hardware independent part of the unit test suite on the
# development PC.  The very same test bodies are executed on the ESP32 by
# test/test_clock_core (Unity).
#
# Usage:  bash test/host/run_host_tests.sh [output-directory]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUT_DIR="${1:-${ROOT_DIR}/build/host}"

# --- pick a C++ compiler ----------------------------------------------------
if command -v g++ >/dev/null 2>&1; then
    CXX="${CXX:-g++}"
elif command -v clang++ >/dev/null 2>&1; then
    CXX="${CXX:-clang++}"
elif python -c "import ziglang" >/dev/null 2>&1; then
    CXX="python -m ziglang c++"
else
    echo "No C++ compiler found (install g++/clang++ or 'pip install ziglang')." >&2
    exit 1
fi

mkdir -p "${OUT_DIR}"

echo "== compiling the host test runner with ${CXX}"
${CXX} -std=c++17 -Wall -Wextra -Wno-nullability-completeness \
    -I "${ROOT_DIR}/main/core" \
    -I "${ROOT_DIR}/main/drivers" \
    -I "${ROOT_DIR}/test/test_clock_core/main" \
    -I "${ROOT_DIR}/test/host" \
    "${ROOT_DIR}/test/host/host_test_main.cpp" \
    "${ROOT_DIR}/test/host/unity_lite.cpp" \
    "${ROOT_DIR}"/main/core/*.cpp \
    "${ROOT_DIR}/main/drivers/Stopwatch.cpp" \
    -o "${OUT_DIR}/host_tests"

echo "== running the host test runner"
"${OUT_DIR}/host_tests"
