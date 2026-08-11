# Function Library

> The primary function library for the Directive Utilities plugin.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilFunctionLibrary.h`

---

## Get Child Classes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static void GetChildClasses(const UClass* BaseClass, bool bRecursive, TArray<UClass*>& DerivedClasses);
```

Returns a list of classes derived from the given base class (not limited to Actors), exposing the built-in GetDerivedClasses function to Blueprints.

| Parameter | Type | Description |
|-----------|------|-------------|
| BaseClass | `const UClass*` | The base class to get the derived classes for. |
| bRecursive | `bool` | Whether to include derived classes of derived classes. |
| DerivedClasses | `TArray<UClass*>&` | The list of derived classes. |

**Returns:** `DerivedClasses` (out param): the list of derived classes.

## Copy Text To Clipboard
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static void CopyTextToClipboard(const FText& Text);
```

Copies the provided text to the clipboard.

| Parameter | Type | Description |
|-----------|------|-------------|
| Text | `const FText&` | The text to copy to the clipboard. |

## Copy String To Clipboard
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static void CopyStringToClipboard(const FString& String);
```

Copies the provided string to the clipboard.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to copy to the clipboard. |

## Get Text From Clipboard
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static FText GetTextFromClipboard();
```

Gets the content from the clipboard as FText.

**Returns:** The text from the clipboard.

## Get String From Clipboard
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static FString GetStringFromClipboard();
```

Gets the content from the clipboard as an FString.

**Returns:** The content from the clipboard as a string.

## Clear Clipboard
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static void ClearClipboard();
```

Clears the clipboard.

## Get Project Version
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static FString GetProjectVersion();
```

Gets the project version as a string.

**Returns:** The project version as a string.

## Is Running In Editor
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static bool IsRunningInEditor();
```

Returns true if the game is running in the Unreal Editor. Returns false in packaged/standalone builds.

**Returns:** True if running in the editor.

## Get World Type
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static EDirectiveUtilWorldType GetWorldType(const UObject* WorldContextObject);
```

Gets the type of world associated with the supplied context. The world context pin is filled automatically when the calling Blueprint provides one.

| Value | Description |
|-------|-------------|
| Unknown | The context is null, has no world, or reports an unsupported world type. |
| None | The world has no assigned type. |
| Game | A game world, including packaged and standalone games. |
| Editor | A world being edited. |
| Play In Editor | A PIE world. |
| Editor Preview | A preview world created by an editor tool. |
| Game Preview | A game preview world. |
| Game RPC | A minimal game RPC world. |
| Inactive | An editor world that is loaded but not currently being edited. |

## Get Build Configuration Type
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static EDirectiveUtilBuildConfiguration GetBuildConfigurationType();
```

Gets the running application's build configuration as an enum: Unknown, Debug, Debug Game, Development, Shipping, or Test. Unlike Unreal's string-returning `Get Build Configuration` node, this value can connect directly to `Switch on EDirectiveUtilBuildConfiguration`.

## Get Build Target Type
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static EDirectiveUtilBuildTargetType GetBuildTargetType();
```

Gets the target that produced the running executable: Unknown, Game, Server, Client, Editor, or Program. This reports the build target, not the world's current network mode.

## Has Command Line Switch
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static bool HasCommandLineSwitch(const FString& Switch);
```

Checks whether a switch (e.g. `MySwitch` matching `-MySwitch`) was passed on the process command line. Matching is case-insensitive.

| Parameter | Type | Description |
|-----------|------|-------------|
| Switch | `const FString&` | The switch name, without the leading dash. |

**Returns:** True if the switch is present.

## Get Command Line Option
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static bool GetCommandLineOption(const FString& Key, FString& OutValue);
```

Reads a key=value option (e.g. `MyKey` matching `-MyKey=Value`) from the process command line. Matching is case-insensitive; quoted values are returned without the quotes.

| Parameter | Type | Description |
|-----------|------|-------------|
| Key | `const FString&` | The key name, without the leading dash or trailing equals sign. |
| OutValue | `FString&` | [out] The option's value, or empty if the key is missing. |

**Returns:** True if the key was present.

## Start Stopwatch
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility|Profiling`

```cpp
static bool StartStopwatch(FName Key, bool bRestartIfRunning = false);
```

Starts a process-wide keyed stopwatch using monotonic real time. A stopwatch may start and stop on different threads. Calls are synchronized, do not require a world, and are unaffected by pause, time dilation, and level travel.

| Parameter | Type | Description |
|-----------|------|-------------|
| Key | `FName` | The name used to stop this stopwatch. `None` is invalid. |
| bRestartIfRunning | `bool` | Replaces the start time when this key is already active. When false, an active stopwatch is left unchanged. |

**Returns:** True when the stopwatch was started or restarted. False when the key is `None`, or when the key is already active and restart is disabled.

## Stop Stopwatch
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility|Profiling`

```cpp
static bool StopStopwatch(FName Key, double& ElapsedMilliseconds);
```

Stops and consumes the active stopwatch for a key. Different keys can overlap and stop in any order.

| Parameter | Type | Description |
|-----------|------|-------------|
| Key | `FName` | The key passed to Start Stopwatch. |
| ElapsedMilliseconds | `double` | The elapsed real time in milliseconds, or `0.0` when the key is not active. |

**Returns:** True when an active stopwatch was found. False for `None`, a missing key, or a key that has already been stopped.

### Measuring a before-and-after change

Warm up both code paths before recording them. Start the `Baseline` key, run the original function, and stop it into `BaselineMs`. Repeat with an `Optimized` key and `OptimizedMs`. Collect several samples in the same build and compare their medians:

```text
Speedup = BaselineMs / OptimizedMs
Time reduction (%) = (BaselineMs - OptimizedMs) / BaselineMs * 100
```

The result includes Blueprint dispatch, stopwatch-node, and scheduling overhead. Use the same machine, engine version, build configuration, and inputs for both paths. A single sample is not enough to support a performance claim; use Unreal Insights or the runtime performance runner for release evidence.
