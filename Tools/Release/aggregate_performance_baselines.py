#!/usr/bin/env python3

from __future__ import annotations

import argparse
import csv
import math
import statistics
from pathlib import Path


class BaselineError(RuntimeError):
    pass


def read_results(path: Path) -> tuple[list[str], dict[tuple[str, str, str], list[str]]]:
    metadata: list[str] = []
    rows: dict[tuple[str, str, str], list[str]] = {}
    with path.open(encoding="utf-8-sig", newline="") as source:
        while True:
            position = source.tell()
            line = source.readline()
            if not line.startswith("#"):
                source.seek(position)
                break
            if not line.startswith("#timestamp_utc,") and not line.startswith("#baseline,"):
                metadata.append(line.rstrip("\r\n"))

        reader = csv.reader(source)
        header = next(reader, None)
        expected_header = [
            "benchmark",
            "element_count",
            "parameter",
            "median_ms",
            "min_ms",
            "max_ms",
            "samples",
            "baseline_median_ms",
            "speedup",
            "change_percent",
        ]
        if header != expected_header:
            raise BaselineError(f"Unexpected performance CSV header: {path}")
        for row in reader:
            if len(row) != len(expected_header):
                raise BaselineError(f"Malformed performance row: {path}")
            key = (row[0], row[1], row[2])
            if key in rows:
                raise BaselineError(f"Duplicate performance row {key}: {path}")
            try:
                median = float(row[3])
                minimum = float(row[4])
                maximum = float(row[5])
                samples = int(row[6])
            except ValueError as error:
                raise BaselineError(f"Invalid performance value for {key}: {path}") from error
            if (
                not all(math.isfinite(value) for value in (median, minimum, maximum))
                or minimum < 0.0
                or minimum > median
                or median > maximum
                or samples <= 0
            ):
                raise BaselineError(f"Invalid performance value for {key}: {path}")
            rows[key] = row
    if not rows:
        raise BaselineError(f"Performance CSV has no results: {path}")
    return metadata, rows


def aggregate(paths: list[Path], output_path: Path) -> None:
    if len(paths) < 3 or len(paths) % 2 == 0:
        raise BaselineError("Provide an odd number of at least three baseline files.")

    metadata, first_rows = read_results(paths[0])
    all_rows = [first_rows]
    for path in paths[1:]:
        current_metadata, current_rows = read_results(path)
        if current_metadata != metadata:
            raise BaselineError(f"Performance metadata does not match: {path}")
        if current_rows.keys() != first_rows.keys():
            raise BaselineError(f"Performance rows do not match: {path}")
        all_rows.append(current_rows)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as destination:
        for line in metadata:
            destination.write(f"{line}\n")
        destination.write(f"#aggregate_runs,{len(paths)}\n")
        writer = csv.writer(destination, lineterminator="\n")
        writer.writerow([
            "benchmark",
            "element_count",
            "parameter",
            "median_ms",
            "min_ms",
            "max_ms",
            "samples",
            "baseline_median_ms",
            "speedup",
            "change_percent",
        ])
        for key, first_row in first_rows.items():
            medians = [float(rows[key][3]) for rows in all_rows]
            minimums = [float(rows[key][4]) for rows in all_rows]
            maximums = [float(rows[key][5]) for rows in all_rows]
            sample_counts = [int(rows[key][6]) for rows in all_rows]
            writer.writerow([
                first_row[0],
                first_row[1],
                first_row[2],
                f"{statistics.median(medians):.9f}",
                f"{min(minimums):.9f}",
                f"{max(maximums):.9f}",
                sum(sample_counts),
                "",
                "",
                "",
            ])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("inputs", nargs="+", type=Path)
    arguments = parser.parse_args()
    aggregate(arguments.inputs, arguments.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
