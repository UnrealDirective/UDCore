# Runtime test host

This project runs the Directive Utilities automation suite in Unreal Editor and a packaged Development game. It is not included in packaged plugin releases. Verification is local only; the repository does not use hosted CI.

Packaged runs execute runtime function and async-task tests, then check the reflected API and module boundaries. Editor-only and uncooked modules must remain absent from the packaged game.

On macOS or Linux:

```sh
Tests/RuntimeHost/Scripts/run-unix.sh "/Users/Shared/Epic Games/UE_5.8"
```

On Windows:

```powershell
Tests\RuntimeHost\Scripts\run-windows.ps1 "C:\Program Files\Epic Games\UE_5.8"
```

Run the command once per supported engine installation. Each runner creates a clean project under `Build/RuntimeHost`, builds the editor target, runs editor automation, packages a Development game, and runs automation from that package.

The local release gate runs this project for UE 5.6, 5.7, and 5.8, then runs the packaged Shipping benchmark on UE 5.8.

On macOS, the UE 5.8 packaged test process starts with `-LLM`. The installed 5.8 runtime can crash while disabling its low-level memory tracker before log initialization. Keeping the tracker enabled avoids that engine startup fault. This changes only the test command; it does not change the packaged plugin or game.

The runner copies runtime-compatible test sources into a game-only host module. The plugin descriptor and released packages remain unchanged.
