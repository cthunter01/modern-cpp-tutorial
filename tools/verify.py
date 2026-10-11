#!/usr/bin/env python3
"""The author-side check of the whole tutorial. Not needed to work through it.

    python3 tools/verify.py                  check the working tree
    python3 tools/verify.py --pristine       check a clean export of the last commit (the real test:
                                             a tree with your own answers in it fails the stub checks)
    python3 tools/verify.py --trees clang-debug,asan --jobs 8

What it establishes, per build tree:

    clang-debug, gcc-debug   every reference solution compiles with warnings as errors and passes its
                             tests; every untouched exercise builds and FAILS its tests; every example
                             prints what the chapter quotes; every experiment ends as the chapter says
    asan                     solutions are clean under AddressSanitizer + UndefinedBehaviorSanitizer;
                             the experiments that need a sanitizer report produce it
    tidy                     solutions and examples are clang-tidy clean, and clang-tidy does reach them
    tsan                     the chapters labelled `concurrency`, repeated

and for the text: every listing is the compiled source (tools/docs.py), links resolve, and all C++ is
clang-format clean. Every command is printed before it runs, so any step can be repeated by hand.
Python standard library only.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ElementTree
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ALL_TREES = ["clang-debug", "gcc-debug", "asan", "tidy", "tsan"]
PLAIN_TREES = {"clang-debug", "gcc-debug"}
TSAN_REPEATS = 20


class Report:
    def __init__(self) -> None:
        self.lines: list[tuple[bool, str]] = []

    def add(self, passed: bool, text: str) -> None:
        self.lines.append((passed, text))
        print(("ok    " if passed else "FAIL  ") + text, flush=True)

    @property
    def failed(self) -> bool:
        return any(not passed for passed, _ in self.lines)


def run(command: list[str], cwd: Path, log: Path | None = None) -> int:
    print("$ " + " ".join(command), flush=True)
    if log is None:
        return subprocess.run(command, cwd=cwd, check=False).returncode
    with log.open("w", encoding="utf-8") as out:
        return subprocess.run(command, cwd=cwd, stdout=out, stderr=subprocess.STDOUT, check=False).returncode


def export_pristine(revision: str) -> Path:
    """`git archive` of a revision into build/verify/src: what a fresh clone contains."""
    target = ROOT / "build" / "verify" / "src"
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True)
    archive = subprocess.run(["git", "-C", str(ROOT), "archive", "--format=tar", revision],
                             capture_output=True, check=False)
    if archive.returncode != 0:
        sys.exit(f"git archive {revision} failed: {archive.stderr.decode()}")
    subprocess.run(["tar", "-x", "-C", str(target)], input=archive.stdout, check=True)
    return target


def source_files(source: Path) -> list[str]:
    listed = subprocess.run(
        ["git", "-C", str(ROOT), "ls-files", "--cached", "--others", "--exclude-standard", "*.cpp", "*.h"],
        capture_output=True, text=True, check=False)
    if listed.returncode == 0 and listed.stdout.strip():
        return [name for name in listed.stdout.splitlines() if (source / name).is_file()]
    return [str(path.relative_to(source)) for pattern in ("*.cpp", "*.h") for path in source.rglob(pattern)
            if "build" not in path.relative_to(source).parts]


def junit_counts(path: Path) -> dict[str, tuple[int, int]]:
    """Per test group (the name's prefix): tests run and tests failed."""
    groups = {"sol": "solutions", "stub": "stub checks", "eg": "examples", "x": "experiments",
              "selfcheck": "self-check", "tidy_canary": "self-check"}
    counts: dict[str, tuple[int, int]] = {}
    for case in ElementTree.parse(path).getroot().iter("testcase"):
        name = case.get("name", "")
        group = next((label for prefix, label in groups.items() if name.startswith(prefix)), "other")
        bad = case.get("status") != "run" or case.find("failure") is not None
        ran, failed = counts.get(group, (0, 0))
        counts[group] = (ran + 1, failed + (1 if bad else 0))
    return counts


def has_concurrency_chapters(source: Path) -> bool:
    return any("concurrency" in path.read_text(encoding="utf-8")
               for path in source.glob("chapters/*/CMakeLists.txt"))


def check_tree(source: Path, tree: str, jobs: int, report: Report, logs: Path) -> None:
    build = source / "build" / tree
    if run(["cmake", "--preset", tree], source, logs / f"{tree}-configure.log") != 0:
        report.add(False, f"{tree}: configure (see {logs / (tree + '-configure.log')})")
        return

    # Compile in parallel first; the tests then find everything built and only run.
    targets = ["all", "solutions", "experiments"]
    if tree in PLAIN_TREES:
        targets.append("exercises")   # in a pristine tree every stub has to build
    if tree == "tsan":
        targets = ["all", "solutions"]
    log = logs / f"{tree}-build.log"
    built = run(["cmake", "--build", "--preset", tree, "-t", *targets], source, log)
    report.add(built == 0, f"{tree}: build of {', '.join(targets)}" + ("" if built == 0 else f" (see {log})"))

    junit = build / "verify.xml"
    command = ["ctest", "--test-dir", str(build), "-LE", "^exercise$", "--no-tests=error",
               "--output-on-failure", "-j", str(jobs), "--output-junit", str(junit)]
    if tree == "tsan":
        command += ["-L", "^concurrency$", "--repeat", f"until-fail:{TSAN_REPEATS}"]
    log = logs / f"{tree}-test.log"
    tested = run(command, source, log)
    if not junit.exists():
        report.add(False, f"{tree}: ctest produced no results (see {log})")
        return
    for group, (ran, failed) in sorted(junit_counts(junit).items()):
        report.add(failed == 0, f"{tree}: {group} {ran - failed}/{ran}" + ("" if failed == 0 else f" (see {log})"))
    if tested != 0 and not report.failed:
        report.add(False, f"{tree}: ctest failed (see {log})")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("--pristine", nargs="?", const="HEAD", metavar="REV",
                        help="check a clean export of REV (default HEAD) instead of the working tree")
    parser.add_argument("--trees", default=",".join(ALL_TREES), help="comma-separated presets to check")
    parser.add_argument("--jobs", "-j", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--logger", type=Path, default=ROOT.parent / "logger",
                        help="the logger repository, for checking quoted logger code")
    args = parser.parse_args()

    started = time.monotonic()
    source = export_pristine(args.pristine) if args.pristine else ROOT
    logs = ROOT / "build" / "verify" / "logs"
    logs.mkdir(parents=True, exist_ok=True)
    report = Report()

    if not args.pristine:
        changed = subprocess.run(["git", "-C", str(ROOT), "status", "--porcelain", "--", "chapters/*/exercises"],
                                 capture_output=True, text=True, check=False).stdout.strip()
        if changed:
            print("note: chapters/*/exercises has changes. If they are your answers, the stub checks will "
                  "fail; use --pristine to check the committed tutorial instead.")

    files = source_files(source)
    formatted = run(["clang-format", "--dry-run", "--Werror", *files], source, logs / "format.log")
    report.add(formatted == 0, f"format: {len(files)} C++ files are clang-format clean"
               + ("" if formatted == 0 else f" (see {logs / 'format.log'})"))

    trees = [tree for tree in args.trees.split(",") if tree]
    if "tsan" in trees and not has_concurrency_chapters(source):
        trees.remove("tsan")
        print("note: no chapter is labelled `concurrency` yet; the tsan tree has nothing to check.")
    for tree in trees:
        check_tree(source, tree, args.jobs, report, logs)

    # After the trees: the documentation check reads the manifest a configured tree writes.
    docs = run([sys.executable, str(source / "tools" / "docs.py"), "check", "--logger", str(args.logger)],
               source, logs / "docs.log")
    report.add(docs == 0, "docs: listings are the compiled sources, links and references resolve"
               + ("" if docs == 0 else f" (see {logs / 'docs.log'})"))

    minutes = (time.monotonic() - started) / 60
    print("\n" + "=" * 78)
    for passed, text in report.lines:
        print(("ok    " if passed else "FAIL  ") + text)
    print(f"{'FAILED' if report.failed else 'all checks passed'} in {minutes:.1f} min "
          f"({'pristine export of ' + args.pristine if args.pristine else 'working tree'})")
    return 1 if report.failed else 0


if __name__ == "__main__":
    sys.exit(main())
