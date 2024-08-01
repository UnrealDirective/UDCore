# UDCore

UDCore is an open-source Unreal Engine plugin developed by Dylan "Tezenari" Amos as part of the Unreal Directive. It provides a variety of quality-of-life functionalities to enhance the development experience.

This plugin will be updated sporadically with new functionality.

## Requirements
- **Software**: Unreal Engine 5.3 ~ 5.4
- **Platform**: Windows

## Features

- **AI Utilities**: Simplified AI task management.
- **String Manipulation**: Functions to handle and manipulate strings.
- **Text Utilities**: Functions to check and manipulate text.
- **Editor Utilities:** Enhanced functionality for the Editor Actor Subsystem

## Installation

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

- ## Usage

  ### AI Utilities

  - **Async Move to Location:**

    ```cpp
    UUDAT_MoveToLocation* Action = UUDAT_MoveToLocation::MoveToLocation(WorldContextObject, Controller, Destination, AcceptanceRadius, bDebugLineTrace);
    ```

  ### String Manipulation

  - **Contains Letters:**

    ```cpp
    bool bHasLetters = UUDCoreFunctionLibrary::ContainsLetters("ExampleString");
    ```

  - **Contains Numbers:**

    ```cpp
    bool bHasNumbers = UUDCoreFunctionLibrary::ContainsNumbers("Example123");
    ```

  - **Filter Characters:**

    ```cpp
    FString FilteredString = UUDCoreFunctionLibrary::FilterCharacters("Example123!@#", true, true, true, true);
    ```

  ### Text Utilities

  - **Is Not Empty:**

    ```cpp
    bool bIsNotEmpty = UUDCoreFunctionLibrary::IsNotEmpty(FText::FromString("Example"));
    ```

  ### Editor Actor Subsystem

  - **Focus Actors In Viewport:**

    ```cpp
    TArray<AActor*> ActorsToFocus;
    // Populate ActorsToFocus with actors
    UUDCoreEditorActorSubsystem::FocusActorsInViewport(ActorsToFocus, true);
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
    // Populate ActorsToFilter with actors
    UUDCoreEditorActorSubsystem::FilterActorsByName(ActorsToFilter, FilteredActors, "ExampleName", EUDInclusivity::Include);
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
