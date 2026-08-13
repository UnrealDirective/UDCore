# Compatibility

Use this page to decide whether the plugin fits a project before adding it to source control or a build pipeline.

## Engine versions

Directive Utilities follows a rolling **"latest three engine versions"** support policy. The currently supported range is **Unreal Engine 5.6 - 5.8**.

| Engine | Status | Notes |
|--------|--------|-------|
| 5.8 | Supported | Local editor, packaged Development, and packaged Shipping behavior and performance gates. |
| 5.7 | Supported | Local editor and packaged Development gate. |
| 5.6 | Supported | Local editor and packaged Development gate. |

UE 5.5 and earlier are **not supported**. The workaround for the `UEditorActorSubsystem` regression tracked in [#6](https://github.com/UnrealDirective/DirectiveUtilities/issues/6) was removed when 5.5 dropped out of the support window.

The repository descriptor omits `EngineVersion` so Git installations work across the supported range. Release packages set the engine version in their copied descriptor.

The project uses a local release gate instead of hosted CI. Run `Tools/Release/run-local-release-gate.sh` on macOS or Linux, or `Tools/Release/run-local-release-gate.ps1` on Windows, before publishing. The publishing gate requires a clean working tree and records evidence against its exact commit. On Windows, pass `-IncludeLinux` to compile the plugin for Linux with Epic's v25 and v26 cross-toolchains. This checks Linux compilation; packaged Linux execution still requires a Linux Unreal installation.

## Platforms

| Platform | Runtime | Editor |
|----------|---------|--------|
| Win64 | Yes | Yes |
| Mac | Yes | Yes |
| Linux | Yes | Yes |

## Module layout

| Module | Type | Ships in packaged game? |
|--------|------|--------------------------|
| `DirectiveUtilitiesRuntime` | Runtime | Yes: this is the only module that ships. |
| `DirectiveUtilitiesBlueprintNodes` | UncookedOnly | No: Blueprint compiler support only. |
| `DirectiveUtilitiesEditor` | Editor | No: editor builds only. |
| `DirectiveUtilitiesTests` | DeveloperTool (Editor target only) | No: automation tests, excluded from all packaged games. |

## Build Targets

- Runtime gameplay code should depend on `DirectiveUtilitiesRuntime`.
- Editor tooling should depend on `DirectiveUtilitiesEditor`.
- Packaged-game targets must not depend on `DirectiveUtilitiesEditor` or `DirectiveUtilitiesTests`.
- The test module is for editor automation only and is not part of the installed runtime surface.

## Languages

Core functionality is written in C++ and exposed to **Blueprints** and **Python**, so it is usable from any of the three.
