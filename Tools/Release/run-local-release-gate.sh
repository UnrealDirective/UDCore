#!/usr/bin/env bash

set -euo pipefail

SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "$SCRIPT_ROOT/../.." && pwd)"

if [[ "$#" -eq 0 ]]; then
	if [[ "$(uname -s)" != "Darwin" ]]; then
		echo "Usage: run-local-release-gate.sh <UE_5.6> <UE_5.7> <UE_5.8>" >&2
		exit 2
	fi
	ENGINE_ROOTS=(
		"/Users/Shared/Epic Games/UE_5.6"
		"/Users/Shared/Epic Games/UE_5.7"
		"/Users/Shared/Epic Games/UE_5.8"
	)
else
	ENGINE_ROOTS=("$@")
fi

if [[ "${#ENGINE_ROOTS[@]}" -ne 3 ]]; then
	echo "Provide the UE 5.6, 5.7, and 5.8 engine roots in that order." >&2
	exit 2
fi

EXPECTED_VERSIONS=("UE_5.6" "UE_5.7" "UE_5.8")
for INDEX in 0 1 2; do
	if [[ "$(basename "${ENGINE_ROOTS[$INDEX]}")" != "${EXPECTED_VERSIONS[$INDEX]}" ]]; then
		echo "Expected ${EXPECTED_VERSIONS[$INDEX]} at argument $((INDEX + 1)): ${ENGINE_ROOTS[$INDEX]}" >&2
		exit 2
	fi
done

cd "$REPOSITORY_ROOT"
python3 -m unittest Tests/Packaging/test_package_fab.py Tests/Release/test_check_release.py
python3 Tools/Release/check_release.py
python3 Tools/Packaging/package_fab.py --check
git diff --check

if rg -n -i 'co-authored-by:|generated (with|by)|chatgpt|openai|claude|copilot|delve|seamless|robust|leverage|comprehensive|streamline|—|–' \
	--glob '!Build/**' --glob '!Binaries/**' --glob '!Intermediate/**' \
	--glob '!Tools/Release/run-local-release-gate.*' .; then
	echo "Release text contains an attribution or generated-content marker." >&2
	exit 1
fi

for ENGINE_ROOT in "${ENGINE_ROOTS[@]}"; do
	Tests/RuntimeHost/Scripts/run-unix.sh "$ENGINE_ROOT" Development
done

PERFORMANCE_ROOT="$REPOSITORY_ROOT/Build/Performance/ReleaseGate"
PERFORMANCE_PROJECT="$REPOSITORY_ROOT/Build/RuntimeHost/UE_5.8/Project/DirectiveUtilitiesRuntimeHost.uproject"
Tests/Performance/run-runtime-benchmarks.sh \
	"${ENGINE_ROOTS[2]}" "$PERFORMANCE_PROJECT" "$PERFORMANCE_ROOT/warmup.csv"
Tests/Performance/run-runtime-benchmarks.sh \
	"${ENGINE_ROOTS[2]}" "$PERFORMANCE_PROJECT" "$PERFORMANCE_ROOT/baseline.csv"
Tests/Performance/run-runtime-benchmarks.sh \
	"${ENGINE_ROOTS[2]}" "$PERFORMANCE_PROJECT" "$PERFORMANCE_ROOT/candidate.csv" \
	"$PERFORMANCE_ROOT/baseline.csv"

Tests/RuntimeHost/Scripts/run-unix.sh "${ENGINE_ROOTS[2]}" Shipping
echo "Local release gate passed."
