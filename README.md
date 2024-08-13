# UDCore

**UDCore** (or **Unreal Directive Core**), is an open-source [Unreal Engine](https://www.unrealengine.com/) plugin that's designed from the ground-up to provide quality-of-life functionalities to enhance the development experience.

This plugin was started by [Dylan "Tezenari" Amos](https://dylanamos.com) as part of the [Unreal Directive](https://unrealdirective.com) initiative to empower Unreal Engine developers with knowledge and tools to better allow them to build amazing things.

UDCore's philosophy revolves around the following --
- **Best Practices** - All functionality built into UDCore should be efficient in execution, easy-to-read, and follow best development practices to allow developers to easily learn from, copy or extended.
- **Workflow Efficiency** - Each function and tool added to UDCore is designed to improve the speed and efficiency of development workflows. This involves exposing hidden functionality from the Unreal Engine editor, adding new functionality, or simplifying existing functionality.
- **Universal** - UDCore functionality should primarily be established use C++ and be exposed to Blueprints & Python.

This plugin will be updated sporadically with new functionality.

[![Read The Documentation](https://img.shields.io/badge/Read%20The%20Documentation-blue)](https://udcore.unrealdirective.com/)

## Requirements
- **Software**: Unreal Engine 5.3 ~ 5.4
- **Platform**: Windows

## Features

- **Async Tasks** - New 
- **String/Text Utilities** - Functions to handle and manipulate strings and text.
- **Editor Actor Subsystem** - A plethora of utilities and filters to that improves upon the Editor Actor Subsystem.
- **Enhanced Input** - New functions to simplify and improve Enhanced Input system.

## Installation

Instruction on how to install the UDCore plugin. I recommend checking out the [Quick Start](https://udcore.unrealdirective.com/docs/getting-started/quickstart) page over on the [documentation](https://udcore.unrealdirective.com/) website.

### Option 1: Plugin Downloader

UDCore can be installed directly by using the [Plugin Downloader](https://www.unrealengine.com/marketplace/en-US/product/plugin-downloader) plugin on the marketplace.

1. **Install [Plugin Downloader](https://www.unrealengine.com/marketplace/en-US/product/plugin-downloader) from the Unreal Engine Marketplace**

2. **Enable the `Plugin Downloader` plugin**

   1. Open your Unreal Engine project.
   2. Go to `Edit` -> `Plugins`.
   3. Search for `Plugin Downloader` and enable it.
   4. Restart the Unreal Engine editor.

3. **Download `UDCore`**

   1. Go to `Edit` -> `Plugins` 
   2. Click on `Download` in the upper left of the `Plugins` window
   3. Enter the following info in the `Download Plugin` window
      1. **User:** UnrealDirective
      2. **Repo:** UDCore
      3. **Branch:** main

   4. Click on `Download` in the bottom right of the `Download Plugin` window
   5. Wait for download to complete
   6. Restart the Unreal Engine editor when prompted

4. **Enable the `UDCore` plugin**

   1. Open your Unreal Engine project.
   2. Go to `Edit` -> `Plugins`.
   3. Search for `Plugin Downloader` and enable it.
   4. Restart the Unreal Engine editor.

### Option 2: Git Clone

1. **Clone the repository:**
   
   ```sh
   git clone https://github.com/UnrealDirective/UDCore.git
   ```
   
2. **Copy the plugin to your Unreal Engine project:**
   - Navigate to your Unreal Engine project's `Plugins` directory.
   - Copy the `UDCore` folder into the `Plugins` directory.

3. **Enable the plugin:**
   - Open your Unreal Engine project.
   - Go to `Edit` > `Plugins`.
   - Search for `UDCore` and enable it.
   - Restart the Unreal Engine editor.

## Usage

Here are examples on how you can go about using some of the functions in UDCore. For more detailed information, please check out the [documentation](https://udcore.unrealdirective.com/docs/features/overview).

### AI Utilities

- **Async Move to Location:**

  
  
  ```cpp
  .cpp
  #include "AI/UDAT_MoveToLocation.h"
  #include "GameFramework/Controller.h"
  #include "GameFramework/Actor.h"
  
  void AExampleCharacter::MovePlayer()
  {
  	UWorld* World = GetWorld();
  	AController* Controller = GetController();
      const FVector Destination(100.0f, 200.0f, 300.0f);
      constexpr float AcceptanceRadius = 50.0f;
      constexpr bool bDebugLineTrace = true;
  
      UUDAT_MoveToLocation* MoveToLocationTask = UUDAT_MoveToLocation::MoveToLocation(
          World,
          Controller,
          Destination,
          AcceptanceRadius,
          bDebugLineTrace);
  
      if (MoveToLocationTask)
      {
          MoveToLocationTask->Completed.AddDynamic(this, &ThisClass::OnMoveToLocationCompleted);
      }
  }
  
  void ATestChar::OnMoveToLocationCompleted(bool bSuccess)
  {
      // Called when UUDAT_MoveToLocation has completed with either a success or fail.
  	// Add your logic here.
  }
  ```

### String Manipulation

- **Contains Letters:**

  ```cpp
  FString StringToCheck = "Example123"
  bool bHasLetters = UUDCoreFunctionLibrary::ContainsLetters(StringToCheck);
  ```

- **Contains Numbers:**

  ```cpp
  FString StringToCheck = "Example123"
  bool bHasNumbers = UUDCoreFunctionLibrary::ContainsNumbers(StringToCheck);
  ```

- **Filter Characters:**

  ```c++
  FString StringToCheck = "Example 123 !@#"
  bool bFilterOutLetters = false;
  bool bFilterOutNumbers = false;
  bool bFilterOutSpecialCharacters = true;
  bool bFilterOutSpaces = true;
  
  // "Example 123" !@# would become "Example123"
  FString FilteredString = UUDCoreFunctionLibrary::FilterCharacters(
  	StringToCheck,
      bFilterOutLetters,
      bFilterOutNumbers,
      bFilterOutSpecialCharacters,
      bFilterOutSpaces);
  ```

### Text Utilities

- **Is Not Empty:**

  ```cpp
  FText TextToCheck = "Example123"
  bool bIsNotEmpty = UUDCoreFunctionLibrary::IsNotEmpty(TextToCheck);
  ```

### Editor Actor Subsystem

- **Focus Actors In Viewport:**

  ```cpp
  TArray<AActor*> ActorsToFocus;
  bool bFocusInstantly = true;
  
  // Populate ActorsToFocus with actors
  UUDCoreEditorActorSubsystem::FocusActorsInViewport(ActorsToFocus, bFocusInstantly);
  ```

- **Get All Level Classes:**

  ```cpp
  TArray<UClass*> LevelClasses = UUDCoreEditorActorSubsystem::GetAllLevelClasses();
  ```

- **Filter Static Mesh Actors:**

  ```cpp
  TArray<AStaticMeshActor*> StaticMeshActors;
  TArray<AActor*> ActorsToFilter;
  
  // Populate ActorsToFilter with actors
  UUDCoreEditorActorSubsystem::FilterStaticMeshActors(StaticMeshActors, ActorsToFilter);
  ```

- **Filter Actors By Name:**

  ```cpp
  TArray<AActor*> FilteredActors;
  TArray<AActor*> ActorsToFilter;
  FString ActorNameToFind = "ExampleName";
  
  // Populate ActorsToFilter with actors
  UUDCoreEditorActorSubsystem::FilterActorsByName(ActorsToFilter, FilteredActors, ActorNameToFind, EUDInclusivity::Include);
  ```

- **Filter Actors By Class:**

  ```cpp
  TArray<AActor*> FilteredActors;
  TArray<AActor*> ActorsToFilter;
  
  // Populate ActorsToFilter with actors
  UUDCoreEditorActorSubsystem::FilterActorsByClass(ActorsToFilter, FilteredActors, AStaticMeshActor::StaticClass(), EUDInclusivity::Include);
  ```



## Contributing

We welcome contributions to enhance the functionality of UDCore. Please follow these steps to contribute:

1. Fork the repository.
2. Create a new branch (`git checkout -b feature/YourFeature`).
3. Commit your changes (`git commit -am 'Add new feature'`).
4. Push to the branch (`git push origin feature/YourFeature`).
5. Create a new Pull Request.

## License

UDCore is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Support

For support, please visit our [GitHub Issues](https://github.com/UnrealDirective/UDCore/issues) page.

## Authors

- Unreal Directive - [Website](https://unrealdirective.com)
- Dylan "Tezenari" Amos - [Website](https://dylanamos.com)

## Acknowledgments

- Special thanks to the Unreal Engine community for their continuous support and contributions.
