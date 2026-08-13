#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import json
import platform
import subprocess
from datetime import datetime, timezone
from pathlib import Path


class EvidenceError(RuntimeError):
    pass


def run_git(repository_root: Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(repository_root), *arguments],
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout.strip()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def csv_metadata(path: Path) -> dict[str, str]:
    metadata: dict[str, str] = {}
    with path.open(encoding="utf-8-sig") as source:
        for line in source:
            if not line.startswith("#"):
                break
            key, separator, value = line[1:].rstrip("\r\n").partition(",")
            if separator:
                metadata[key] = value
    return metadata


def require_file(path: Path) -> Path:
    if not path.is_file():
        raise EvidenceError(f"Required release evidence is missing: {path}")
    return path


def artifact_path(path: Path, repository_root: Path) -> str:
    try:
        return path.relative_to(repository_root).as_posix()
    except ValueError:
        return path.as_posix()


def resolve_fab_validation_root(
    repository_root: Path,
    fab_validation_root: Path | None,
) -> Path:
    return fab_validation_root or repository_root / "Build" / "FabValidation"


def linux_cross_compile_artifacts(package_root: Path) -> list[Path]:
    intermediate_root = package_root / "Intermediate" / "Build" / "Linux"
    artifacts: list[Path] = []
    for configuration in ("Development", "Shipping"):
        configuration_root = intermediate_root / "x64" / "UnrealGame" / configuration
        objects = sorted(configuration_root.rglob("*.o"))
        if not objects:
            raise EvidenceError(
                f"Linux {configuration} cross-compiled objects are missing: {configuration_root}"
            )
        artifacts.extend(objects)
    return artifacts


def validate_editor_report(path: Path) -> dict[str, int]:
    report = json.loads(require_file(path).read_text(encoding="utf-8-sig"))
    counts = {
        name: int(report.get(name, -1))
        for name in ("succeeded", "succeededWithWarnings", "failed", "notRun")
    }
    if counts["succeeded"] <= 0 or any(
        counts[name] != 0 for name in ("succeededWithWarnings", "failed", "notRun")
    ):
        raise EvidenceError(f"Editor automation report is not clean: {path}")
    return counts


def collect_artifacts(
    repository_root: Path,
    revision_label: str,
    linux_package_base: Path | None = None,
    fab_validation_root: Path | None = None,
) -> tuple[list[Path], dict[str, dict[str, int]]]:
    build_root = repository_root / "Build"
    artifacts: list[Path] = []
    editor_results: dict[str, dict[str, int]] = {}
    for engine_version in ("UE_5.6", "UE_5.7", "UE_5.8"):
        development_root = build_root / "RuntimeHost" / engine_version / "Development"
        report_path = development_root / "Reports" / "Editor" / "index.json"
        editor_results[f"{engine_version}/Development"] = validate_editor_report(report_path)
        game_log = require_file(development_root / "GameTests.log")
        if "TEST COMPLETE. EXIT CODE: 0" not in game_log.read_text(encoding="utf-8", errors="replace"):
            raise EvidenceError(f"Packaged runtime tests did not finish cleanly: {game_log}")
        artifacts.extend((report_path, game_log))

    shipping_root = build_root / "RuntimeHost" / "UE_5.8" / "Shipping"
    shipping_report = shipping_root / "Reports" / "Editor" / "index.json"
    editor_results["UE_5.8/Shipping"] = validate_editor_report(shipping_report)
    artifacts.append(shipping_report)
    for name in ("shipping-append-comparison.csv", "shipping-smoke.csv"):
        path = require_file(shipping_root / "Performance" / name)
        metadata = csv_metadata(path)
        if metadata.get("configuration") != "Shipping":
            raise EvidenceError(f"Shipping evidence has the wrong configuration: {path}")
        if metadata.get("revision") != revision_label:
            raise EvidenceError(f"Shipping evidence is not bound to {revision_label}: {path}")
        artifacts.append(path)

    performance_path = require_file(build_root / "Performance" / "ReleaseGate" / "candidate.csv")
    performance_metadata = csv_metadata(performance_path)
    if performance_metadata.get("revision") != revision_label:
        raise EvidenceError(f"Performance evidence is not bound to {revision_label}: {performance_path}")
    artifacts.append(performance_path)

    descriptor = json.loads(
        (repository_root / "DirectiveUtilities.uplugin").read_text(encoding="utf-8-sig")
    )
    version = descriptor["VersionName"]
    validation_root = resolve_fab_validation_root(repository_root, fab_validation_root)
    for engine_version in ("5.6", "5.7", "5.8"):
        artifacts.append(require_file(
            build_root / "Fab" / f"DirectiveUtilities-{version}-UE{engine_version}-Fab.zip"
        ))
        package_root = validation_root / f"UE_{engine_version}" / "Package"
        artifacts.append(require_file(package_root / "DirectiveUtilities.uplugin"))
        binaries = sorted(path for path in (package_root / "Binaries").rglob("*") if path.is_file())
        if not binaries:
            raise EvidenceError(f"Compiled Fab binaries are missing: {package_root / 'Binaries'}")
        artifacts.extend(binaries)

    if linux_package_base:
        for engine_version in ("5.6", "5.7", "5.8"):
            package_root = linux_package_base / f"UE_{engine_version}"
            artifacts.append(require_file(package_root / "DirectiveUtilities.uplugin"))
            artifacts.extend(linux_cross_compile_artifacts(package_root))

    return artifacts, editor_results


def validation_matrix(host_platform: str, include_linux_cross_compile: bool) -> list[dict[str, object]]:
    runtime_platform = {"Darwin": "Mac", "Windows": "Win64"}.get(host_platform, host_platform)
    matrix: list[dict[str, object]] = []
    for engine_version in ("5.6", "5.7", "5.8"):
        matrix.append({
            "engine": engine_version,
            "platform": runtime_platform,
            "configuration": "Development",
            "checks": ["editor_automation", "packaged_runtime"],
        })
        matrix.append({
            "engine": engine_version,
            "platform": runtime_platform,
            "configuration": "BuildPlugin",
            "checks": ["generated_fab_archive"],
        })
    matrix.append({
        "engine": "5.8",
        "platform": runtime_platform,
        "configuration": "Shipping",
        "checks": ["editor_automation", "packaged_smoke", "packaged_benchmark"],
    })
    if include_linux_cross_compile:
        for engine_version in ("5.6", "5.7", "5.8"):
            matrix.append({
                "engine": engine_version,
                "platform": "Linux",
                "configuration": "BuildPlugin",
                "checks": ["cross_compile"],
            })
    return matrix


def write_manifest(
    repository_root: Path,
    output_path: Path,
    allow_dirty: bool,
    linux_package_base: Path | None = None,
    fab_validation_root: Path | None = None,
) -> None:
    revision = run_git(repository_root, "rev-parse", "HEAD")
    dirty = bool(run_git(repository_root, "status", "--porcelain", "--untracked-files=all"))
    if dirty and not allow_dirty:
        raise EvidenceError("Release evidence must be recorded from a clean working tree.")
    revision_label = f"{revision}-dirty" if dirty else revision
    artifacts, editor_results = collect_artifacts(
        repository_root,
        revision_label,
        linux_package_base,
        fab_validation_root,
    )
    descriptor = json.loads(
        (repository_root / "DirectiveUtilities.uplugin").read_text(encoding="utf-8-sig")
    )
    manifest = {
        "schemaVersion": 1,
        "status": "passed",
        "pluginVersion": descriptor["VersionName"],
        "revision": revision,
        "dirty": dirty,
        "host": platform.platform(),
        "createdAtUtc": datetime.now(timezone.utc).isoformat(),
        "validationMatrix": validation_matrix(platform.system(), linux_package_base is not None),
        "editorResults": editor_results,
        "artifacts": [
            {
                "path": artifact_path(path, repository_root),
                "bytes": path.stat().st_size,
                "sha256": sha256_file(path),
            }
            for path in sorted(set(artifacts))
        ],
    }
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Record local Directive Utilities release evidence.")
    parser.add_argument("--repository-root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--allow-dirty", action="store_true")
    parser.add_argument("--linux-package-base", type=Path)
    parser.add_argument("--fab-validation-root", type=Path)
    options = parser.parse_args()
    try:
        write_manifest(
            options.repository_root.resolve(),
            options.output.resolve(),
            options.allow_dirty,
            options.linux_package_base.resolve() if options.linux_package_base else None,
            options.fab_validation_root.resolve() if options.fab_validation_root else None,
        )
    except (EvidenceError, OSError, KeyError, json.JSONDecodeError, subprocess.CalledProcessError) as error:
        print(error)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
