#!/usr/bin/env python3
"""Keeps the chapters honest: every listing is code that is compiled, every link leads somewhere.

    python3 tools/docs.py check  [--logger PATH]    report every difference, exit 1 if there is one
    python3 tools/docs.py update [--logger PATH]    rewrite the tagged listings from their sources

A listing in a chapter is a fenced block with a tag on the line above it:

    <!-- file: chapters/01-views/examples/e01_trim.cpp#trim -->       a region of a file of this repository
    <!-- file: chapters/01-views/experiments/x01_dangling.expect -->  a whole file
    <!-- logger: src/Level.cpp:21-34 -->                              lines of logger at LOGGER_COMMIT

`update` replaces the content of the block; `check` compares it. A region is delimited in the source by
two identical marker lines, `//! [name]`. The block gets the lines between them, without markers of other
regions and without the indentation the lines have in common.

A `cpp` or `cmake` block without a tag is an error, unless the line above it says why it is not compiled:

    <!-- not-compiled: a signature only -->

Logger listings are checked against the logger repository (a sibling directory by default). Without it
they are skipped, and `check` says so. Python standard library only.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LOGGER_COMMIT = "49f42d1"  # version 0.1.0, the one the rebuild guide is written against
CHECKED_LANGUAGES = {"cpp", "cmake"}

TAG = re.compile(r"^<!--\s*(file|logger|not-compiled):\s*(.*?)\s*-->\s*$")
FENCE = re.compile(r"^(\s*)(`{3,})(\S*)\s*$")
MARKER = re.compile(r"^\s*(?://|#)!\s*\[([^\]]+)\]\s*$")
LINK = re.compile(r"(?<!\!)\[[^\]]*\]\(([^)\s]+)\)")
CODE_SPAN = re.compile(r"`([^`\n]+)`")
TARGET = re.compile(r"^(?:ch|sol|stub)([0-9a-z]{2})_([dp]\d\d)(?:\.\w+)?$|^(?:eg|x)([0-9a-z]{2})_(\d\d)$")
CHAPTER_DIRS = ("examples/", "experiments/", "exercises/", "tests/")
ROOT_DIRS = ("chapters/", "solutions/", "support/", "cmake/", "tools/")


class Problems:
    def __init__(self) -> None:
        self.items: list[str] = []
        self.notices: list[str] = []

    def add(self, where: Path, line: int, text: str) -> None:
        self.items.append(f"{where.relative_to(ROOT)}:{line}: {text}")

    def notice(self, text: str) -> None:
        if text not in self.notices:
            self.notices.append(text)


def dedent(lines: list[str]) -> list[str]:
    indents = [len(line) - len(line.lstrip(" ")) for line in lines if line.strip()]
    cut = min(indents, default=0)
    return [line[cut:] if line.strip() else "" for line in lines]


def region_of(text: str, region: str | None) -> list[str] | None:
    """The lines of `text` that a tag asks for, or None when the region does not exist."""
    lines = text.splitlines()
    if region is None:
        return [line for line in lines if not MARKER.match(line)]
    inside = False
    found = False
    result: list[str] = []
    for line in lines:
        marker = MARKER.match(line)
        if marker:
            if marker.group(1) == region:
                if inside:
                    return dedent(result)
                inside = True
                found = True
            continue
        if inside:
            result.append(line)
    return None if not found else dedent(result)


def logger_lines(logger: Path, spec: str) -> list[str] | str:
    """Lines of a file in logger at LOGGER_COMMIT; a string is an error message."""
    match = re.fullmatch(r"(.+?):(\d+)(?:-(\d+))?", spec)
    if not match:
        return f"cannot read '{spec}' as path:first-last"
    path, first, last = match.group(1), int(match.group(2)), int(match.group(3) or match.group(2))
    shown = subprocess.run(
        ["git", "-C", str(logger), "show", f"{LOGGER_COMMIT}:{path}"],
        capture_output=True, text=True, check=False)
    if shown.returncode != 0:
        return f"logger has no {path} at {LOGGER_COMMIT}"
    lines = shown.stdout.splitlines()
    if last > len(lines) or first < 1 or first > last:
        return f"{path} has {len(lines)} lines, not {first}-{last}"
    return dedent(lines[first - 1:last])


def slug(heading: str) -> str:
    """GitHub's anchor for a heading."""
    text = re.sub(r"`", "", heading.strip().lower())
    text = re.sub(r"[^\w\- ]", "", text)
    return text.replace(" ", "-")


def anchors_of(path: Path) -> set[str]:
    anchors: set[str] = set()
    in_fence = False
    for line in path.read_text(encoding="utf-8").splitlines():
        if FENCE.match(line):
            in_fence = not in_fence
        elif not in_fence and line.startswith("#"):
            anchors.add(slug(line.lstrip("#")))
    return anchors


def load_manifest() -> set[str] | None:
    """Every target the build knows, from the manifest of whichever tree was configured."""
    ids: set[str] = set()
    found = False
    for manifest in sorted(ROOT.glob("build/*/tut-manifest.tsv")):
        found = True
        for line in manifest.read_text(encoding="utf-8").splitlines():
            parts = line.split("\t")
            if len(parts) >= 2:
                ids.add(parts[1])
    return ids if found else None


def preset_names() -> set[str]:
    presets = json.loads((ROOT / "CMakePresets.json").read_text(encoding="utf-8"))
    names: set[str] = set()
    for kind in ("configurePresets", "buildPresets", "testPresets", "workflowPresets"):
        names.update(preset["name"] for preset in presets.get(kind, []))
    return names


def process(path: Path, logger: Path | None, update: bool, problems: Problems,
            manifest: set[str] | None, presets: set[str]) -> None:
    original = path.read_text(encoding="utf-8").splitlines()
    out: list[str] = []
    index = 0
    pending: tuple[str, str, int] | None = None  # the tag waiting for its block
    while index < len(original):
        line = original[index]
        tag = TAG.match(line)
        fence = FENCE.match(line)
        if tag:
            pending = (tag.group(1), tag.group(2), index + 1)
            out.append(line)
            index += 1
            continue
        if not fence:
            if line.strip():
                if pending and pending[0] != "not-compiled":
                    problems.add(path, pending[2], "this tag is not followed by a fenced block")
                pending = None
                check_prose(path, index + 1, line, problems, manifest, presets)
            out.append(line)
            index += 1
            continue

        # A fenced block: find its end.
        indent, ticks, language = fence.group(1), fence.group(2), fence.group(3)
        end = index + 1
        while end < len(original) and original[end].strip() != ticks:
            end += 1
        body = original[index + 1:end]
        wanted: list[str] | None = None
        if pending is None:
            if language in CHECKED_LANGUAGES:
                problems.add(path, index + 1,
                             f"a {language} block with no tag: tag its source, or say <!-- not-compiled: why -->")
        elif pending[0] == "file":
            name, _, region = pending[1].partition("#")
            source = ROOT / name
            if not source.is_file():
                problems.add(path, pending[2], f"{name} does not exist")
            else:
                wanted = region_of(source.read_text(encoding="utf-8"), region or None)
                if wanted is None:
                    problems.add(path, pending[2], f"{name} has no region [{region}]")
        elif pending[0] == "logger":
            if logger is None:
                problems.notice("logger listings were not checked: no logger repository found (--logger PATH)")
            else:
                result = logger_lines(logger, pending[1])
                if isinstance(result, str):
                    problems.add(path, pending[2], result)
                else:
                    wanted = result
        if wanted is not None:
            wanted = [indent + text if text else "" for text in wanted]
            if body != wanted:
                if update:
                    body = wanted
                else:
                    problems.add(path, index + 1,
                                 f"listing differs from {pending[1]} (run tools/docs.py update)")
        out.append(line)
        out.extend(body)
        if end < len(original):
            out.append(original[end])
        index = end + 1
        pending = None

    if update and out != original:
        path.write_text("\n".join(out) + "\n", encoding="utf-8")
        print(f"updated {path.relative_to(ROOT)}")


def check_prose(path: Path, number: int, line: str, problems: Problems,
                manifest: set[str] | None, presets: set[str]) -> None:
    for target in LINK.findall(line):
        if re.match(r"[a-z]+:", target):  # http:, https:, mailto:
            continue
        name, _, anchor = target.partition("#")
        linked = (path.parent / name).resolve() if name else path
        if not linked.exists():
            problems.add(path, number, f"link to {target}: {name} does not exist")
        elif anchor and linked.suffix == ".md" and anchor not in anchors_of(linked):
            problems.add(path, number, f"link to {target}: no heading gives the anchor #{anchor}")

    for span in CODE_SPAN.findall(line):
        token = span.strip()
        if " " not in token and "*" not in token and "<" not in token:
            candidate: Path | None = None
            if token.startswith(CHAPTER_DIRS) and path.parent.parent.name == "chapters":
                candidate = path.parent / token
            elif token.startswith(ROOT_DIRS):
                candidate = ROOT / token
            if candidate is not None and not candidate.exists():
                # The files of a design problem are the reader's to create.
                if not re.search(r"exercises/p\d\d_", token):
                    problems.add(path, number, f"`{token}` does not exist")
            if TARGET.match(token):
                base = re.sub(r"\.\w+$", "", token)
                base = re.sub(r"^(sol|stub)", "ch", base)
                if manifest is None:
                    problems.notice("target names were not checked: configure a preset first")
                elif base not in manifest:
                    problems.add(path, number, f"`{token}` is not a target the build knows")
        for preset in re.findall(r"--preset[ =]([\w-]+)", span):
            if preset not in presets:
                problems.add(path, number, f"--preset {preset}: no such preset")


def markdown_files() -> list[Path]:
    files = [path for path in ROOT.rglob("*.md") if "build" not in path.relative_to(ROOT).parts]
    return sorted(files)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("command", choices=["check", "update"])
    parser.add_argument("--logger", type=Path, default=None,
                        help="the logger repository (default: ../logger next to this repository)")
    args = parser.parse_args()

    logger = args.logger or ROOT.parent / "logger"
    if not (logger / ".git").exists():
        logger = None

    problems = Problems()
    manifest = load_manifest()
    presets = preset_names()
    files = markdown_files()
    for path in files:
        process(path, logger, args.command == "update", problems, manifest, presets)
    # `sh` blocks are not prose, but the presets they name must exist all the same.
    for path in files:
        for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if not CODE_SPAN.search(line):
                for preset in re.findall(r"--preset[ =]([\w-]+)", line):
                    if preset not in presets:
                        problems.add(path, number, f"--preset {preset}: no such preset")

    # A header that is given to the reader (not a stub: its name has no dNN_ or pNN_ in front) exists
    # twice, once for the exercises and once for the solutions. The two must not drift apart.
    for given in sorted(ROOT.glob("chapters/*/exercises/*.h")):
        if re.match(r"[dp]\d\d_", given.name):
            continue
        twin = ROOT / "solutions" / given.parent.parent.name / given.name
        if not twin.is_file():
            problems.add(given, 1, f"the given header has no copy at {twin.relative_to(ROOT)}")
        elif twin.read_bytes() != given.read_bytes():
            problems.add(given, 1, f"differs from its copy {twin.relative_to(ROOT)}")

    for notice in problems.notices:
        print(f"note: {notice}")
    for item in problems.items:
        print(item)
    print(f"docs: {len(files)} files, {len(problems.items)} problem(s)")
    return 1 if problems.items else 0


if __name__ == "__main__":
    sys.exit(main())
