# Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from Tools.Release import check_release


class ReleaseIdentityTest(unittest.TestCase):
    def make_repository(self, version: int = 7, version_name: str = "2.2.0") -> Path:
        temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(temporary_directory.cleanup)
        repository_root = Path(temporary_directory.name)
        (repository_root / "DirectiveUtilities.uplugin").write_text(
            json.dumps({"Version": version, "VersionName": version_name}),
            encoding="utf-8",
        )
        (repository_root / "CHANGELOG.md").write_text(
            f"# Changelog\n\n## [{version_name}] - 2026-08-11\n",
            encoding="utf-8",
        )
        return repository_root

    def git_result(self, arguments: list[str], returncode: int = 0) -> subprocess.CompletedProcess[str]:
        command = arguments[1:]
        if command[:2] == ["tag", "--list"]:
            stdout = "v2.1.0\n"
        elif command[:2] == ["show", "v2.1.0:DirectiveUtilities.uplugin"]:
            stdout = json.dumps({"Version": 6, "VersionName": "2.1.0"})
        elif command[:2] == ["rev-parse", "HEAD"]:
            stdout = "candidate\n"
        elif command[:3] == ["rev-list", "-n", "1"]:
            stdout = "released\n"
        elif command[:2] == ["status", "--porcelain"]:
            stdout = ""
        else:
            stdout = ""
        return subprocess.CompletedProcess(arguments, returncode, stdout, "")

    def test_new_release_version_passes(self) -> None:
        repository_root = self.make_repository()
        with patch.object(
            check_release.subprocess,
            "run",
            side_effect=lambda arguments, **_: self.git_result(arguments),
        ):
            check_release.validate_release_identity(repository_root)

    def test_reused_release_version_is_rejected(self) -> None:
        repository_root = self.make_repository(6, "2.1.0")
        with patch.object(
            check_release.subprocess,
            "run",
            side_effect=lambda arguments, **_: self.git_result(arguments),
        ):
            with self.assertRaisesRegex(
                check_release.ReleaseCheckError,
                "Untagged changes cannot package",
            ):
                check_release.validate_release_identity(repository_root)

    def test_numeric_version_must_advance(self) -> None:
        repository_root = self.make_repository(6, "2.2.0")
        with patch.object(
            check_release.subprocess,
            "run",
            side_effect=lambda arguments, **_: self.git_result(arguments),
        ):
            with self.assertRaisesRegex(
                check_release.ReleaseCheckError,
                "Numeric Version must be greater",
            ):
                check_release.validate_release_identity(repository_root)

    def test_dirty_released_version_is_rejected(self) -> None:
        repository_root = self.make_repository(6, "2.1.0")

        def dirty_repository(arguments: list[str], **_: object) -> subprocess.CompletedProcess[str]:
            result = self.git_result(arguments)
            command = arguments[1:]
            if command[:2] in (["rev-parse", "HEAD"], ["rev-list", "-n"]):
                return subprocess.CompletedProcess(arguments, 0, "released\n", "")
            if command[:2] == ["status", "--porcelain"]:
                return subprocess.CompletedProcess(arguments, 0, " M DirectiveUtilities.uplugin\n", "")
            return result

        with patch.object(check_release.subprocess, "run", side_effect=dirty_repository):
            with self.assertRaisesRegex(
                check_release.ReleaseCheckError,
                "Untagged changes cannot package",
            ):
                check_release.validate_release_identity(repository_root)

    def test_changelog_heading_is_required(self) -> None:
        repository_root = self.make_repository()
        (repository_root / "CHANGELOG.md").write_text("# Changelog\n", encoding="utf-8")
        with self.assertRaisesRegex(
            check_release.ReleaseCheckError,
            r"dated \[2.2.0\] release heading",
        ):
            check_release.validate_release_identity(repository_root)


if __name__ == "__main__":
    unittest.main()
