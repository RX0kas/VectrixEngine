#!/usr/bin/env python3
"""Change the engine's version everywhere it is written.

Usage: set_version.py <X.Y.Z | major | minor | patch> [--dry-run] [--force]
       set_version.py --release [--date DD-MM-YYYY] [--dry-run]

The first form sets the version (or bumps one part of the current one, the lower parts going back to 0) in
CMakeLists.txt, Vectrix/src/Vectrix/Core/version.h, Doxyfile and CLAUDE.md, and adds an empty
"vX.Y.Z - (unreleased)" section at the top of Vectrix/src/dox/changelog.dox.
The second form dates the current version's changelog section (today by default), once it is published.

Every file is checked before any is written: if one of them doesn't have the expected line, nothing changes.
"""
import argparse
import datetime
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
VERSION_H = "Vectrix/src/Vectrix/Core/version.h"
CHANGELOG = "Vectrix/src/dox/changelog.dox"

# (file, pattern, replacement): each pattern must match exactly once. {v} is "X.Y.Z", {major} {minor} {patch} its parts
REPLACEMENTS = [
    ("CMakeLists.txt", r"(project\(VectrixWorkspace VERSION )\d+\.\d+\.\d+", r"\g<1>{v}"),
    (VERSION_H, r'(#define VC_VERSION )"\d+\.\d+\.\d+"', r'\g<1>"{v}"'),
    (VERSION_H, r"(#define VC_VERSION_MAJOR )\d+", r"\g<1>{major}"),
    (VERSION_H, r"(#define VC_VERSION_MINOR )\d+", r"\g<1>{minor}"),
    (VERSION_H, r"(#define VC_VERSION_PATCH )\d+", r"\g<1>{patch}"),
    ("Doxyfile", r"(PROJECT_NUMBER\s*=\s*)\d+\.\d+\.\d+", r"\g<1>{v}"),
    ("CLAUDE.md", r"(The root project is `VectrixWorkspace` version `)\d+\.\d+\.\d+", r"\g<1>{v}"),
]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def current_version() -> tuple[int, int, int]:
    match = re.search(r'#define VC_VERSION "(\d+)\.(\d+)\.(\d+)"', read(VERSION_H))
    if not match:
        sys.exit(f"error: no VC_VERSION in {VERSION_H}")
    return tuple(int(part) for part in match.groups())


def target_version(arg: str, current: tuple[int, int, int]) -> tuple[int, int, int]:
    major, minor, patch = current
    if arg == "major":
        return major + 1, 0, 0
    if arg == "minor":
        return major, minor + 1, 0
    if arg == "patch":
        return major, minor, patch + 1
    match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", arg)
    if not match:
        sys.exit(f"error: '{arg}' is not X.Y.Z, major, minor or patch")
    return tuple(int(part) for part in match.groups())


def changelog_with_section(text: str, v: str) -> str:
    anchor = "v" + v.replace(".", "_")
    if f"@section {anchor} " in text:
        return text  # Already there (e.g. the version was set back and forth)
    first = text.find(" * @section ")
    if first == -1:
        sys.exit(f"error: no @section in {CHANGELOG}")
    section = (f" * @section {anchor} v{v} - (unreleased)\n"
               f" *\n"
               f" * ---\n"
               f" *\n")
    return text[:first] + section + text[first:]


def write_all(files: dict[str, str], dry_run: bool) -> None:
    for path, text in files.items():
        if text == read(path):
            continue
        print(f"{'would update' if dry_run else 'updated'} {path}")
        if not dry_run:
            (ROOT / path).write_text(text, encoding="utf-8")


def set_version(arg: str, dry_run: bool, force: bool) -> None:
    current = current_version()
    major, minor, patch = target_version(arg, current)
    if (major, minor, patch) <= current and not force:
        sys.exit(f"error: {major}.{minor}.{patch} is not after the current {'.'.join(map(str, current))} "
                 f"(--force to set it anyway)")
    v = f"{major}.{minor}.{patch}"

    files: dict[str, str] = {}
    for path, pattern, replacement in REPLACEMENTS:
        text = files.get(path, read(path))
        text, count = re.subn(pattern, replacement.format(v=v, major=major, minor=minor, patch=patch), text)
        if count != 1:
            sys.exit(f"error: {path}: expected one match of {pattern!r}, found {count}; nothing was changed")
        files[path] = text
    files[CHANGELOG] = changelog_with_section(read(CHANGELOG), v)

    write_all(files, dry_run)
    print(f"version {'.'.join(map(str, current))} -> {v}")


def release(date: str, dry_run: bool) -> None:
    v = ".".join(map(str, current_version()))
    text = read(CHANGELOG)
    heading = f"@section v{v.replace('.', '_')} v{v} - (unreleased)"
    if heading not in text:
        sys.exit(f"error: {CHANGELOG} has no unreleased section for v{v}")
    write_all({CHANGELOG: text.replace(heading, f"@section v{v.replace('.', '_')} v{v} - ({date})", 1)}, dry_run)
    print(f"v{v} dated {date}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Change the Vectrix engine version in every file that holds it.")
    parser.add_argument("version", nargs="?", help="X.Y.Z, or major/minor/patch to bump that part")
    parser.add_argument("--release", action="store_true", help="date the current version's changelog section")
    parser.add_argument("--date", default=datetime.date.today().strftime("%d-%m-%Y"), help="DD-MM-YYYY, for --release")
    parser.add_argument("--dry-run", action="store_true", help="show the files that would change, write nothing")
    parser.add_argument("--force", action="store_true", help="allow a version that isn't after the current one")
    args = parser.parse_args()

    if args.release == (args.version is not None):
        parser.error("give either a version or --release")
    if args.release:
        if not re.fullmatch(r"\d{2}-\d{2}-\d{4}", args.date):
            parser.error("--date is DD-MM-YYYY")
        release(args.date, args.dry_run)
    else:
        set_version(args.version, args.dry_run, args.force)
    return 0


if __name__ == "__main__":
    sys.exit(main())
