#!/usr/bin/env bash

set -euo pipefail

SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "$SCRIPT_ROOT/../.." && pwd)"

if [[ "$#" -ne 3 ]]; then
	echo "Usage: verify-fab-artifacts.sh <UE_5.6> <UE_5.7> <UE_5.8>" >&2
	exit 2
fi

ENGINE_ROOTS=("$@")
ENGINE_VERSIONS=("5.6" "5.7" "5.8")
case "$(uname -s)" in
	Darwin) TARGET_PLATFORM="Mac" ;;
	Linux) TARGET_PLATFORM="Linux" ;;
	*) echo "Unsupported host platform: $(uname -s)" >&2; exit 2 ;;
esac
PLUGIN_VERSION="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1], encoding="utf-8-sig"))["VersionName"])' "$REPOSITORY_ROOT/DirectiveUtilities.uplugin")"
FAB_ROOT="$REPOSITORY_ROOT/Build/Fab"
VALIDATION_ROOT="$REPOSITORY_ROOT/Build/FabValidation"
PACKAGE_ARGUMENTS=()
if [[ "${DIRECTIVE_UTILITIES_ALLOW_DIRTY_RELEASE:-0}" == "1" ]]; then
	PACKAGE_ARGUMENTS+=(--allow-dirty)
fi

mkdir -p "$FAB_ROOT" "$VALIDATION_ROOT"
python3 "$REPOSITORY_ROOT/Tools/Packaging/package_fab.py" \
	--output-dir "$FAB_ROOT" "${PACKAGE_ARGUMENTS[@]}"

for INDEX in 0 1 2; do
	ENGINE_VERSION="${ENGINE_VERSIONS[$INDEX]}"
	ENGINE_ROOT="${ENGINE_ROOTS[$INDEX]}"
	RUN_UAT="$ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
	ARCHIVE_PATH="$FAB_ROOT/DirectiveUtilities-${PLUGIN_VERSION}-UE${ENGINE_VERSION}-Fab.zip"
	VERSION_ROOT="$VALIDATION_ROOT/UE_${ENGINE_VERSION}"
	EXTRACT_ROOT="$VERSION_ROOT/Extracted"
	PACKAGE_ROOT="$VERSION_ROOT/Package"

	if [[ ! -x "$RUN_UAT" ]]; then
		echo "RunUAT not found: $RUN_UAT" >&2
		exit 1
	fi
	if [[ ! -f "$ARCHIVE_PATH" ]]; then
		echo "Fab archive not found: $ARCHIVE_PATH" >&2
		exit 1
	fi

	rm -rf "$EXTRACT_ROOT" "$PACKAGE_ROOT"
	mkdir -p "$EXTRACT_ROOT"
	python3 -m zipfile -e "$ARCHIVE_PATH" "$EXTRACT_ROOT"
	PLUGIN_PATH="$EXTRACT_ROOT/DirectiveUtilities/DirectiveUtilities.uplugin"
	if [[ ! -f "$PLUGIN_PATH" ]]; then
		echo "Extracted plugin descriptor not found: $PLUGIN_PATH" >&2
		exit 1
	fi

	"$RUN_UAT" BuildPlugin \
		-Plugin="$PLUGIN_PATH" \
		-Package="$PACKAGE_ROOT" \
		-TargetPlatforms="$TARGET_PLATFORM" \
		-Rocket
done

echo "Generated Fab archives compiled successfully."
