# UDCore

UDCore is an open-source Unreal Engine plugin developed by Unreal Directive. It provides a variety of quality-of-life functionalities to enhance the development experience.

## Features

- **AI Utilities**: Simplified AI task management.
- **String Manipulation**: Functions to handle and manipulate strings.
- **Text Utilities**: Functions to check and manipulate text.
- **Debugging Tools**: Enhanced debugging capabilities.

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

## Usage

### AI Utilities

- **Move to Location:**
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

## Acknowledgments

- Special thanks to the Unreal Engine community for their continuous support and contributions.
