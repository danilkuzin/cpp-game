#!/usr/bin/env python3
"""Run clang-tidy's misc-include-cleaner against every project header, in isolation.

Why this exists: misc-include-cleaner only analyzes the main file of a translation unit
(https://github.com/llvm/llvm-project/issues/67550), so neither a normal `cmake --build`
nor `clang-tidy some.cpp` ever reports missing/unused #includes *inside* a .h file — only
in .cpp files. The only way to check a header is to compile it as its own translation
unit (-x c++) and point clang-tidy at that. Doing this by hand needs several easy-to-forget
flags; this script is that invocation, kept in one place instead of re-derived per session.

Usage: python3 scripts/check_header_includes.py   (run from the repo root, after configuring
       the `debug` preset at least once so build/compile_commands.json exists)

A clean run prints nothing and exits 0.

Generated with CLAUDE
"""

import concurrent.futures
import glob
import json
import os
import shlex
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COMPILE_COMMANDS = os.path.join(REPO, "build", "compile_commands.json")
MODULE_DIRS = ["Core", "Domain", "Graphics", "Game", "GameStates", "Ui"]
# Texts is an INTERFACE library with no .cpp of its own, so it has no compile_commands.json
# entry to borrow flags from; GameStates links Texts and covers its include paths.
NO_OWN_CPP_DIRS = {"Texts": "GameStates"}

# Noise inherent to treating a header as its own main file, not real findings:
#   pragma-once-outside-header — every project header uses #pragma once
#   unused-{const-variable,function,variable,parameter} — symbols a header defines for callers
#   look locally unused when nothing in this one-header "TU" calls them.
NOISE_FLAGS = [
    "-Wno-pragma-once-outside-header",
    "-Wno-unused-const-variable",
    "-Wno-unused-function",
    "-Wno-unused-variable",
    "-Wno-unused-parameter",
]


def load_flag_templates():
    with open(COMPILE_COMMANDS) as f:
        entries = json.load(f)
    templates = {}
    for e in entries:
        rel = os.path.relpath(e["file"], REPO)
        for module in MODULE_DIRS:
            if rel.startswith(module + "/") and rel.endswith(".cpp") and module not in templates:
                cmd = shlex.split(e["command"])
                templates[module] = (cmd[1:], e["directory"])  # drop argv[0] (the compiler)
    return templates


def build_jobs(templates):
    jobs = []
    for module in [*MODULE_DIRS, *NO_OWN_CPP_DIRS]:
        source_module = NO_OWN_CPP_DIRS.get(module, module)
        if source_module not in templates:
            continue
        flags, cwd = templates[source_module]
        for header in glob.glob(os.path.join(REPO, module, "*.h")):
            rel = os.path.relpath(header, REPO)
            cmd = [
                "clang-tidy",
                "--extra-arg-before=--driver-mode=g++",
                "--extra-arg-before=-xc++",
                "-checks=-*,misc-include-cleaner",
                header,
                "--",
                *flags,
                *NOISE_FLAGS,
            ]
            jobs.append((rel, cmd, cwd))
    return jobs


def run(job):
    rel, cmd, cwd = job
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd, check=False)
    # clang-tidy's own driver-mode reconciliation prints deprecation/ordering noise for the
    # -x c++ / header-as-main-file trick; strip it rather than teach clang-tidy to suppress it.
    lines = [
        line
        for line in result.stdout.splitlines()
        if "has no effect" not in line
        and "treating 'c-header'" not in line
        and not line.strip().endswith("generated.")
    ]
    return rel, "\n".join(lines)


def main():
    if not os.path.exists(COMPILE_COMMANDS):
        print(f"missing {COMPILE_COMMANDS} — run `cmake --preset debug` first", file=sys.stderr)
        return 2

    jobs = build_jobs(load_flag_templates())
    found_anything = False
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
        for rel, output in pool.map(run, jobs):
            if output.strip():
                found_anything = True
                print(f"=== {rel} ===")
                print(output)

    return 1 if found_anything else 0


if __name__ == "__main__":
    sys.exit(main())
