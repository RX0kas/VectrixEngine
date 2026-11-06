#!/usr/bin/env python3
"""Change the engine's version everywhere it is written.

Usage: set_version.py <X.Y.Z | major | minor | patch> [--dry-run] [--force]
       set_version.py --release [--date DD-MM-YYYY] [--dry-run]

The first form starts a new version (or bumps one part of the current one, the lower parts going back to 0): it
is written in CMakeLists.txt, Vectrix/src/Vectrix/Core/version.h, Doxyfile and CLAUDE.md, marked as a work in
progress (VC_VERSION "X.Y.Z-WIP", VC_VERSION_WIP 1: a build of it says so in its window title), and an empty
"vX.Y.Z - (unreleased)" section is added at the top of Vectrix/src/dox/changelog.dox.
The second form, once the version is ready to publish, drops the WIP mark and dates its changelog section
(today by default).

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
VERSION = r"\d+\.\d+\.\d+(?:-WIP)?"

# (file, pattern, replacement): each pattern must match exactly once. {v} is "X.Y.Z", {shown} the version the
# engine reports ("X.Y.Z-WIP" while in progress), {wip} 1 or 0, {major} {minor} {patch} the parts of {v}
REPLACEMENTS = [
    ("CMakeLists.txt", r"(project\(VectrixWorkspace VERSION )\d+\.\d+\.\d+", r"\g<1>{v}"),  # CMake only takes numbers
    (VERSION_H, rf'(#define VC_VERSION )"{VERSION}"', r'\g<1>"{shown}"'),
    (VERSION_H, r"(#define VC_VERSION_MAJOR )\d+", r"\g<1>{major}"),
    (VERSION_H, r"(#define VC_VERSION_MINOR )\d+", r"\g<1>{minor}"),
    (VERSION_H, r"(#define VC_VERSION_PATCH )\d+", r"\g<1>{patch}"),
    (VERSION_H, r"(#define VC_VERSION_WIP )[01]", r"\g<1>{wip}"),
    ("Doxyfile", rf"(PROJECT_NUMBER\s*=\s*){VERSION}", r"\g<1>{shown}"),
    ("CLAUDE.md", r"(The root project is `VectrixWorkspace` version `)\d+\.\d+\.\d+", r"\g<1>{v}"),
]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def current_version() -> tuple[tuple[int, int, int], bool]:
    """The version in version.h, and whether it is still a work in progress."""
    match = re.search(r'#define VC_VERSION "(\d+)\.(\d+)\.(\d+)(-WIP)?"', read(VERSION_H))
    if not match:
        sys.exit(f"error: no VC_VERSION in {VERSION_H}")
    return tuple(int(part) for part in match.groups()[:3]), match.group(4) is not None


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


def with_version(files: dict[str, str], version: tuple[int, int, int], wip: bool) -> dict[str, str]:
    major, minor, patch = version
    v = f"{major}.{minor}.{patch}"
    values = dict(v=v, shown=f"{v}-WIP" if wip else v, wip=int(wip), major=major, minor=minor, patch=patch)
    for path, pattern, replacement in REPLACEMENTS:
        text = files.get(path, read(path))
        text, count = re.subn(pattern, replacement.format(**values), text)
        if count != 1:
            sys.exit(f"error: {path}: expected one match of {pattern!r}, found {count}; nothing was changed")
        files[path] = text
    return files


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
    current, current_wip = current_version()
    version = target_version(arg, current)
    if version <= current and not force:
        sys.exit(f"error: {'.'.join(map(str, version))} is not after the current {'.'.join(map(str, current))} "
                 f"(--force to set it anyway)")
    if current_wip and not force:
        print(f"warning: {'.'.join(map(str, current))} was never released (still WIP)")
    v = ".".join(map(str, version))

    files = with_version({}, version, wip=True)
    files[CHANGELOG] = changelog_with_section(read(CHANGELOG), v)
    write_all(files, dry_run)
    print(f"version {'.'.join(map(str, current))} -> {v}-WIP")


def release(date: str, dry_run: bool) -> None:
    version, wip = current_version()
    v = ".".join(map(str, version))
    text = read(CHANGELOG)
    heading = f"@section v{v.replace('.', '_')} v{v} - (unreleased)"
    if not wip and heading not in text:
        sys.exit(f"error: v{v} is already released")
    if heading not in text:
        sys.exit(f"error: {CHANGELOG} has no unreleased section for v{v}")

    files = with_version({}, version, wip=False)
    files[CHANGELOG] = text.replace(heading, f"@section v{v.replace('.', '_')} v{v} - ({date})", 1)
    write_all(files, dry_run)
    print(f"v{v} released, dated {date}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Change the Vectrix engine version in every file that holds it.")
    parser.add_argument("version", nargs="?", help="X.Y.Z, or major/minor/patch to bump that part")
    parser.add_argument("--release", action="store_true", help="drop the WIP mark and date the changelog section")
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
