#!/usr/bin/env python3
"""Regression guard for the "Clock::timeval" C++ trap.

Inside ``namespace Clock`` an elaborated type specifier such as
``struct timeval`` declares a *brand new* type (``Clock::timeval``) whenever
the POSIX type is not visible yet.  That type is incompatible with the one
used by ``esp_sntp_time_cb_t``, so the ESP-IDF build fails with the confusing
diagnostic ``struct Clock::timeval``.

The host tool chain hides the problem: ``<ctime>`` - pulled in through
``main/core/DateTime.hpp`` - declares ``::timeval`` globally on mingw, while
newlib (the ESP-IDF tool chain) does not.  A plain type check therefore cannot
reproduce the failure, so this script enforces the source rules that keep the
target build safe:

  1. ``main/drivers/NTPManager.hpp`` and ``main/drivers/NTPManager.cpp``
     include ``<sys/time.h>`` before the name ``timeval`` is used.
  2. Both files always spell the type as ``::timeval`` and never use the bare
     elaborated form ``struct timeval``, which could declare a namespace local
     type again.

Usage:  python test/host/check_timeval_include.py
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TARGETS = [
    ROOT / "main" / "drivers" / "NTPManager.hpp",
    ROOT / "main" / "drivers" / "NTPManager.cpp",
]

INCLUDE = "#include <sys/time.h>"
TIMEVAL = re.compile(r"\btimeval\b")
BARE_STRUCT = re.compile(r"\bstruct\s+timeval\b")


def strip_comments(text: str) -> str:
    """Remove // and /* */ comments so that prose never triggers a rule."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", " ", text)


def check(path: Path) -> list[str]:
    errors: list[str] = []
    raw = path.read_text(encoding="utf-8")
    code = strip_comments(raw)

    include_at = code.find(INCLUDE)
    if include_at < 0:
        errors.append(f"{path.name}: missing '{INCLUDE}'")
        return errors

    first_use = TIMEVAL.search(code)
    if first_use is None:
        errors.append(f"{path.name}: the POSIX type 'timeval' is not used at all")
    elif first_use.start() < include_at:
        errors.append(
            f"{path.name}: '{INCLUDE}' must come before the first use of 'timeval'"
        )

    for match in BARE_STRUCT.finditer(code):
        line = code.count("\n", 0, match.start()) + 1
        errors.append(
            f"{path.name}:{line}: use '::timeval' instead of 'struct timeval' "
            "(an elaborated type specifier inside 'namespace Clock' would "
            "declare a new Clock::timeval type)"
        )

    return errors


def main() -> int:
    problems: list[str] = []
    for path in TARGETS:
        if not path.exists():
            problems.append(f"{path}: file not found")
            continue
        problems.extend(check(path))

    if problems:
        print("== timeval include check FAILED", file=sys.stderr)
        for problem in problems:
            print(f"   {problem}", file=sys.stderr)
        return 1

    print("== timeval include check passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
