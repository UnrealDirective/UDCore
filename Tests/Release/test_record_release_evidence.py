from __future__ import annotations

import hashlib
import tempfile
import unittest
from pathlib import Path

from Tools.Release.record_release_evidence import (
    EvidenceError,
    artifact_path,
    csv_metadata,
    linux_cross_compile_artifacts,
    resolve_fab_validation_root,
    sha256_file,
    validation_matrix,
)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class ReleaseEvidenceTest(unittest.TestCase):
    def test_csv_metadata_stops_at_header(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "result.csv"
            path.write_text(
                "#configuration,Shipping\n#revision,abc123\ncheck,passed\n#ignored,value\n",
                encoding="utf-8",
            )
            self.assertEqual(
                csv_metadata(path),
                {"configuration": "Shipping", "revision": "abc123"},
            )

    def test_sha256_file_hashes_binary_content(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "artifact.bin"
            content = b"directive-utilities-release-evidence"
            path.write_bytes(content)
            self.assertEqual(sha256_file(path), hashlib.sha256(content).hexdigest())

    def test_validation_matrix_records_host_and_linux_checks(self) -> None:
        matrix = validation_matrix("Windows", True)
        self.assertEqual(len(matrix), 10)
        self.assertIn(
            {
                "engine": "5.8",
                "platform": "Win64",
                "configuration": "Shipping",
                "checks": ["editor_automation", "packaged_smoke", "packaged_benchmark"],
            },
            matrix,
        )
        self.assertEqual(
            sum(row["platform"] == "Linux" for row in matrix),
            3,
        )

    def test_artifact_path_keeps_external_paths(self) -> None:
        repository_root = Path("/repo")
        self.assertEqual(
            artifact_path(repository_root / "Build" / "report.json", repository_root),
            "Build/report.json",
        )
        self.assertEqual(
            artifact_path(Path("/artifacts/Linux/plugin.so"), repository_root),
            "/artifacts/Linux/plugin.so",
        )

    def test_fab_validation_root_supports_short_external_paths(self) -> None:
        repository_root = Path("/repo")
        self.assertEqual(
            resolve_fab_validation_root(repository_root, None),
            repository_root / "Build" / "FabValidation",
        )
        self.assertEqual(
            resolve_fab_validation_root(repository_root, Path("/tmp/DUFab")),
            Path("/tmp/DUFab"),
        )

    def test_fab_verifiers_pin_the_host_target_platform(self) -> None:
        windows_script = (
            REPOSITORY_ROOT / "Tools" / "Release" / "verify-fab-artifacts.ps1"
        ).read_text(encoding="utf-8")
        unix_script = (
            REPOSITORY_ROOT / "Tools" / "Release" / "verify-fab-artifacts.sh"
        ).read_text(encoding="utf-8")
        self.assertIn('"-TargetPlatforms=Win64"', windows_script)
        self.assertIn('-TargetPlatforms="$TARGET_PLATFORM"', unix_script)

    def test_windows_runtime_runner_checks_generated_path_length(self) -> None:
        windows_runner = (
            REPOSITORY_ROOT / "Tests" / "RuntimeHost" / "Scripts" / "run-windows.ps1"
        ).read_text(encoding="utf-8")
        self.assertIn("$LongestActionPath.Length -ge 260", windows_runner)

    def test_release_gates_require_two_clean_performance_retries(self) -> None:
        windows_gate = (
            REPOSITORY_ROOT / "Tools" / "Release" / "run-local-release-gate.ps1"
        ).read_text(encoding="utf-8")
        unix_gate = (
            REPOSITORY_ROOT / "Tools" / "Release" / "run-local-release-gate.sh"
        ).read_text(encoding="utf-8")
        for gate in (windows_gate, unix_gate):
            self.assertIn("candidate-attempt-2.csv", gate)
            self.assertIn("candidate-attempt-3.csv", gate)

    def test_linux_evidence_requires_both_configurations(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            package_root = Path(directory)
            for configuration in ("Development", "Shipping"):
                object_path = (
                    package_root
                    / "Intermediate"
                    / "Build"
                    / "Linux"
                    / "x64"
                    / "UnrealGame"
                    / configuration
                    / "DirectiveUtilitiesRuntime"
                    / "Module.cpp.o"
                )
                object_path.parent.mkdir(parents=True, exist_ok=True)
                object_path.write_bytes(configuration.encode())
            self.assertEqual(len(linux_cross_compile_artifacts(package_root)), 2)

    def test_linux_evidence_rejects_a_missing_configuration(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(EvidenceError, "Development"):
                linux_cross_compile_artifacts(Path(directory))


if __name__ == "__main__":
    unittest.main()
