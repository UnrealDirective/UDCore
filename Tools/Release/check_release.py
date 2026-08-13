#!/usr/bin/env python3
# Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
SEMVER_PATTERN = re.compile(r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$")


class ReleaseCheckError(RuntimeError):
    pass


def run_git(repository_root: Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", *arguments],
        cwd=repository_root,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise ReleaseCheckError(result.stderr.strip() or "Git command failed.")
    return result.stdout.strip()


def parse_version(version: str) -> tuple[int, int, int]:
    match = SEMVER_PATTERN.fullmatch(version)
    if not match:
        raise ReleaseCheckError(f"VersionName must be semantic version x.y.z: {version}")
    return tuple(int(part) for part in match.groups())


def validate_release_identity(repository_root: Path) -> None:
    descriptor_path = repository_root / "DirectiveUtilities.uplugin"
    changelog_path = repository_root / "CHANGELOG.md"
    descriptor = json.loads(descriptor_path.read_text(encoding="utf-8-sig"))
    version_name = descriptor.get("VersionName")
    numeric_version = descriptor.get("Version")
    if not isinstance(version_name, str):
        raise ReleaseCheckError("The plugin descriptor must define VersionName.")
    if not isinstance(numeric_version, int) or numeric_version <= 0:
        raise ReleaseCheckError("The plugin descriptor must define a positive numeric Version.")

    current_version = parse_version(version_name)
    changelog = changelog_path.read_text(encoding="utf-8")
    release_heading = re.compile(
        rf"^## \[{re.escape(version_name)}\] - \d{{4}}-\d{{2}}-\d{{2}}$",
        re.MULTILINE,
    )
    if not release_heading.search(changelog):
        raise ReleaseCheckError(
            f"CHANGELOG.md must contain a dated [{version_name}] release heading."
        )

    tags = run_git(repository_root, "tag", "--list", "v[0-9]*", "--sort=-version:refname")
    latest_tag = tags.splitlines()[0] if tags else ""
    if not latest_tag:
        return

    latest_version = parse_version(latest_tag.removeprefix("v"))
    tagged_descriptor = json.loads(
        run_git(repository_root, "show", f"{latest_tag}:DirectiveUtilities.uplugin")
    )
    tagged_numeric_version = tagged_descriptor.get("Version")
    if current_version < latest_version:
        raise ReleaseCheckError(
            f"Descriptor version {version_name} is older than {latest_tag}."
        )
    if current_version > latest_version:
        if not isinstance(tagged_numeric_version, int) or numeric_version <= tagged_numeric_version:
            raise ReleaseCheckError(
                f"Numeric Version must be greater than the value in {latest_tag}."
            )
        return

    head = run_git(repository_root, "rev-parse", "HEAD")
    tagged_commit = run_git(repository_root, "rev-list", "-n", "1", latest_tag)
    dirty_paths = run_git(repository_root, "status", "--porcelain", "--untracked-files=all")
    if head != tagged_commit or dirty_paths:
        raise ReleaseCheckError(
            f"Untagged changes cannot package with the released version {version_name}."
        )


def main() -> int:
    try:
        validate_release_identity(REPOSITORY_ROOT)
    except (OSError, json.JSONDecodeError, ReleaseCheckError) as error:
        print(error, file=sys.stderr)
        return 1
    print("Release identity is valid.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
