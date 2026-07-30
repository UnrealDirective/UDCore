# Runtime performance tests

`Performance.DirectiveUtilities.Runtime` records median, minimum, and maximum execution times for array operations, weighted sampling, natural sorting, medians, and string matching. It also runs same-build comparisons for Unreal's single-item removal, insertion, and append paths against the corresponding Directive Utilities nodes. The workloads vary collection size, duplicate density, sampling ratio, page size, rotation distance, input order, and candidate count. Performance measurements are informational and do not use fixed pass or fail thresholds.

The project used to run the suite must load the current source version of Directive Utilities. Build its Editor target after each implementation change before running the benchmark.

On macOS or Linux:

```sh
Tests/Performance/run-runtime-benchmarks.sh "/Users/Shared/Epic Games/UE_5.8" "/path/to/Project.uproject" "/path/to/results.csv"
```

On Windows:

```powershell
Tests\Performance\run-runtime-benchmarks.ps1 "C:\Program Files\Epic Games\UE_5.8" "C:\path\to\Project.uproject" "C:\path\to\results.csv"
```

Run the command before changing an implementation to capture the baseline. After the change, keep the baseline and choose a different output file:

```sh
Tests/Performance/run-runtime-benchmarks.sh \
  "/Users/Shared/Epic Games/UE_5.8" \
  "/path/to/Project.uproject" \
  "/path/to/optimized.csv" \
  "/path/to/baseline.csv"
```

The optimized CSV includes the baseline median, speedup ratio, and percentage change for each matching workload. Positive percentages indicate faster execution.

Each run also writes `<output-name>-remove-all-comparison.csv`. That file contains the stock and optimized timings, match counts, speedup, and time reduction for integer arrays from 256 through 16,384 elements and string arrays from 256 through 4,096 elements. It covers no-match, single-tail, every-64th, clustered, alternating, and all-match patterns. The every-64th integer case also runs at 1,000 through 1,000,000 elements to trace the scaling curve. Denser patterns stop at 16,384 elements so the stock comparison stays practical. Expect the suite to spend about a minute on the million-element case alone. Inputs are prepared outside the timed region, operation order alternates between samples, and every result is checked for the same return value and ordered survivors.

The `<output-name>-append-comparison.csv` file compares Unreal's generic Blueprint append path with `Append Array Optimized`. Integer arrays run at 1,000, 10,000, 100,000, 250,000, and 1,000,000 source elements with empty and populated targets. String arrays run through 100,000 source elements as a control for the property-aware fallback. Target and source preparation happens outside the timed region, operation order alternates between samples, and the resulting arrays are checked for equality. Self-appending behavior is covered by correctness tests instead of the timed comparison because Unreal's stock implementation emits a false capacity warning after successfully appending an array to itself.

The `<output-name>-insert-comparison.csv` file compares a loop of Unreal `Insert` calls with `Insert Array Optimized` at the front, middle, and end of integer arrays. The `<output-name>-remove-indices-comparison.csv` file compares descending `Remove Index` calls with `Remove At Indices`. Both comparisons run from 256 through 16,384 target elements, alternate execution order, exclude input preparation from the timed region, and verify identical ordered results.

Compare results from the same machine, engine version, build configuration, and host project. The runners use median timing across seven samples to reduce short-lived system noise. Generated results belong under `Build/Performance` and are not committed.

To package the runtime host in Shipping and run the append comparison inside the packaged game:

```sh
Tests/RuntimeHost/Scripts/run-unix.sh "/Users/Shared/Epic Games/UE_5.8" Shipping
```

```powershell
Tests\RuntimeHost\Scripts\run-windows.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -ClientConfiguration Shipping
```

The packaged Shipping comparison covers Boolean, integer, float, vector, string, and object-reference arrays from 1,000 through 1,000,000 source elements. It records whether each reflected element type uses the bulk-copy or property-aware path.

Shipping results are written under `Build/RuntimeHost/<engine-version>/Performance`.
