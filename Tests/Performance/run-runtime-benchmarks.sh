#!/usr/bin/env bash

set -euo pipefail

ENGINE_ROOT="${1:?Usage: run-runtime-benchmarks.sh <engine-root> <project-file> <output-file> [baseline-file]}"
PROJECT_FILE="${2:?Usage: run-runtime-benchmarks.sh <engine-root> <project-file> <output-file> [baseline-file]}"
OUTPUT_FILE="${3:?Usage: run-runtime-benchmarks.sh <engine-root> <project-file> <output-file> [baseline-file]}"
BASELINE_FILE="${4:-}"
TEST_TIMEOUT_SECONDS="${DIRECTIVE_UTILITIES_TEST_TIMEOUT_SECONDS:-1800}"

case "$(uname -s)" in
	Darwin)
		EDITOR_COMMAND="$ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd"
		;;
	Linux)
		EDITOR_COMMAND="$ENGINE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"
		;;
	*)
		echo "Unsupported host platform: $(uname -s)" >&2
		exit 2
		;;
esac

if [[ ! -x "$EDITOR_COMMAND" ]]; then
	echo "Unreal Editor command not found: $EDITOR_COMMAND" >&2
	exit 2
fi

if [[ ! -f "$PROJECT_FILE" ]]; then
	echo "Project file not found: $PROJECT_FILE" >&2
	exit 2
fi

PROJECT_FILE="$(cd "$(dirname "$PROJECT_FILE")" && pwd)/$(basename "$PROJECT_FILE")"
mkdir -p "$(dirname "$OUTPUT_FILE")"
OUTPUT_FILE="$(cd "$(dirname "$OUTPUT_FILE")" && pwd)/$(basename "$OUTPUT_FILE")"
LOG_FILE="${OUTPUT_FILE%.*}.log"
REPORT_DIRECTORY="${OUTPUT_FILE%.*}-report"
rm -rf -- "$REPORT_DIRECTORY"
mkdir -p "$REPORT_DIRECTORY"
COMPARISON_FILE="${OUTPUT_FILE%.*}-remove-all-comparison.csv"
APPEND_COMPARISON_FILE="${OUTPUT_FILE%.*}-append-comparison.csv"
INSERT_COMPARISON_FILE="${OUTPUT_FILE%.*}-insert-comparison.csv"
REMOVE_INDICES_COMPARISON_FILE="${OUTPUT_FILE%.*}-remove-indices-comparison.csv"
REPOSITORY_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
REVISION="$(git -C "$REPOSITORY_ROOT" rev-parse HEAD 2>/dev/null || true)"
if [[ -n "$REVISION" ]] && [[ -n "$(git -C "$REPOSITORY_ROOT" status --porcelain 2>/dev/null)" ]]; then
	REVISION="${REVISION}-dirty"
fi
ARGUMENTS=(
	"$PROJECT_FILE"
	'-ExecCmds=Automation RunTests Performance.DirectiveUtilities.Runtime; Quit'
	'-TestExit=Automation Test Queue Empty'
	"-ReportExportPath=$REPORT_DIRECTORY"
	"-DirectiveUtilitiesPerfOutput=$OUTPUT_FILE"
	"-DirectiveUtilitiesPerfComparisonOutput=$COMPARISON_FILE"
	"-DirectiveUtilitiesPerfAppendComparisonOutput=$APPEND_COMPARISON_FILE"
	"-DirectiveUtilitiesPerfRevision=$REVISION"
	"-abslog=$LOG_FILE"
	-unattended
	-nop4
	-nosplash
	-nosound
	-NullRHI
	-NoEngineAnalytics
	-NoEpicPortal
	'-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False'
)

if [[ -n "$BASELINE_FILE" ]]; then
	if [[ ! -f "$BASELINE_FILE" ]]; then
		echo "Baseline file not found: $BASELINE_FILE" >&2
		exit 2
	fi
	BASELINE_FILE="$(cd "$(dirname "$BASELINE_FILE")" && pwd)/$(basename "$BASELINE_FILE")"
	ARGUMENTS+=("-DirectiveUtilitiesPerfBaseline=$BASELINE_FILE")
fi

set +e
python3 - "$TEST_TIMEOUT_SECONDS" "$EDITOR_COMMAND" "${ARGUMENTS[@]}" <<'PY'
import os, signal, subprocess, sys
timeout = int(sys.argv[1])
process = subprocess.Popen(sys.argv[2:], start_new_session=True)
try:
    raise SystemExit(process.wait(timeout=timeout))
except subprocess.TimeoutExpired:
    os.killpg(process.pid, signal.SIGTERM)
    try:
        process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        process.wait()
    print(f"Runtime performance suite timed out after {timeout} seconds", file=sys.stderr)
    raise SystemExit(124)
PY
EDITOR_EXIT_CODE=$?
set -e

python3 - "$REPORT_DIRECTORY/index.json" <<'PY'
import json, sys
from pathlib import Path
path = Path(sys.argv[1])
if not path.is_file():
    raise SystemExit(f"Performance automation report was not generated: {path}")
report = json.loads(path.read_text(encoding="utf-8-sig"))
tests = report.get("tests", [])
clean = (
    report.get("succeeded") == 1 and report.get("failed") == 0
    and report.get("succeededWithWarnings") == 0 and report.get("notRun") == 0
    and len(tests) == 1 and tests[0].get("fullTestPath") == "Performance.DirectiveUtilities.Runtime"
    and tests[0].get("state") == "Success"
)
if not clean:
    raise SystemExit(f"Performance report is not the exact clean one-test census: {path}")
PY

if [[ "$EDITOR_EXIT_CODE" -ne 0 ]] || [[ ! -f "$OUTPUT_FILE" ]] || [[ ! -f "$COMPARISON_FILE" ]] || [[ ! -f "$APPEND_COMPARISON_FILE" ]] || \
	[[ ! -f "$INSERT_COMPARISON_FILE" ]] || [[ ! -f "$REMOVE_INDICES_COMPARISON_FILE" ]] || \
	! grep -q 'Test Completed. Result={Success} Name={Runtime} Path={Performance.DirectiveUtilities.Runtime}' "$LOG_FILE"; then
	tail -n 100 "$LOG_FILE" >&2
	echo "Runtime performance suite failed. Log: $LOG_FILE" >&2
	exit 1
fi

echo "Runtime performance results: $OUTPUT_FILE"
echo "Remove All comparison results: $COMPARISON_FILE"
echo "Append comparison results: $APPEND_COMPARISON_FILE"
echo "Insert comparison results: $INSERT_COMPARISON_FILE"
echo "Remove At Indices comparison results: $REMOVE_INDICES_COMPARISON_FILE"
echo "Automation log: $LOG_FILE"
