#!/usr/bin/env bash
#
# Type checks the ESP-IDF dependent sources (main/main.cpp, main/core/*.cpp and
# main/drivers/*.cpp) on a development PC that has no ESP-IDF installation.
#
# The ESP-IDF v6 API is replaced by the minimal stubs in test/host/stubs, so
# this script cannot replace a real "idf.py build" - it only guarantees that
# every driver source still compiles cleanly with -Wall -Wextra.
#
# Usage:  bash test/host/typecheck_target.sh
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUT_DIR="${ROOT_DIR}/build/typecheck"

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

status=0
for src in "${ROOT_DIR}/main/main.cpp" "${ROOT_DIR}"/main/core/*.cpp "${ROOT_DIR}"/main/drivers/*.cpp; do
    name="$(echo "${src#"${ROOT_DIR}/"}" | tr '/' '_' | sed 's/\.cpp$/.o/')"
    echo "== type checking ${src#"${ROOT_DIR}/"}"
    if ! ${CXX} -std=c++17 -Wall -Wextra -Wno-nullability-completeness -c "${src}" \
            -I "${ROOT_DIR}/test/host/stubs" \
            -I "${ROOT_DIR}/main/core" \
            -I "${ROOT_DIR}/main/drivers" \
            -I "${ROOT_DIR}/main" \
            -o "${OUT_DIR}/${name}"; then
        status=1
    fi
done

# The Unity test application is checked against the same stubs.
echo "== type checking test/test_clock_core/main/unit_tests.cpp"
if ! ${CXX} -std=c++17 -Wall -Wextra -Wno-nullability-completeness -c \
        "${ROOT_DIR}/test/test_clock_core/main/unit_tests.cpp" \
        -I "${ROOT_DIR}/test/host/stubs" \
        -I "${ROOT_DIR}/main/core" \
        -I "${ROOT_DIR}/main/drivers" \
        -I "${ROOT_DIR}/test/test_clock_core/main" \
        -o "${OUT_DIR}/unit_tests.o"; then
    status=1
fi

if [ "${status}" -eq 0 ]; then
    echo "== every ESP-IDF dependent source compiles cleanly"
else
    echo "== type check FAILED" >&2
fi
exit "${status}"
