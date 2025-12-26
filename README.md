# MFI Controller Support for Geometry Dash (iOS)

A Geode mod that enables full MFi (Made for iPhone/iPad/iPod) game controller support for Geometry Dash on iOS.

## Features

- ✅ Automatic controller detection and connection
- ✅ Support for all MFi-certified controllers (Xbox, PlayStation, etc.)
- ✅ Seamless gameplay integration
- ✅ Configurable button mappings
- ✅ Hot-plugging support (connect/disconnect during gameplay)

## Supported Controllers

This mod supports all MFi-certified game controllers, including:
- Xbox Wireless Controller
- PlayStation DualShock 4 & DualSense
- SteelSeries Nimbus
- Backbone One
- Razer Kishi
- And any other MFi-certified controller

## Default Button Mapping

### Gameplay
- **A Button / Right Trigger / D-pad Up**: Jump/Click
- **B Button / Left Trigger**: Secondary action (for platformer mode, ships, etc.)

### Menus
- **A Button**: Select/Confirm
- **B Button**: Back/Cancel
- **D-pad**: Navigate menus
- **Thumbsticks**: Navigate menus

## Building

### Prerequisites
- macOS with Xcode and iPhone SDK installed
- Geode SDK v4.4.0 or higher
- Geode CLI

### Build Instructions

1. Install Geode iOS binaries:
```bash
geode sdk install-binaries --platform ios
```

2. Build the mod:
```bash
geode build -p ios
```

Or manually:
```bash
cmake -B build -DCMAKE_SYSTEM_NAME=iOS -DGEODE_TARGET_PLATFORM=iOS -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

3. Upload to your iOS device using the Geode Launcher web server or iTunes.

## Installation

1. Install Geode on your iOS device
2. Download the `.geode` file from releases
3. Upload via Geode Launcher's web server or copy to the mods folder
4. Restart Geometry Dash
5. Connect your MFi controller and start playing!

## How It Works

This mod uses Apple's GameController framework to:
1. Monitor for controller connections/disconnections
2. Read input from all supported controller profiles (Extended Gamepad)
3. Translate controller inputs to Geometry Dash's input system
4. Provide low-latency gameplay experience

## Technical Details

The mod hooks into the following Geometry Dash classes:
- `MenuLayer`: Initializes controller support
- `PlayLayer`: Handles gameplay input from the controller
- `PauseLayer`: Enables controller navigation in pause menu

## Requirements

- iOS 13.0 or later (MFi controller support)
- Geometry Dash 2.2074
- Geode Mod Loader

## Troubleshooting

**Controller not detected?**
- Make sure your controller is in pairing mode
- Check Bluetooth settings to ensure the controller is connected
- Restart the game after connecting the controller

**Input lag?**
- Ensure your controller firmware is up to date
- Try reconnecting the controller
- Close other apps that might be using the controller

**Buttons not responding?**
- Check that your controller is MFi-certified
- Try pressing different buttons to see if any respond
- Check the Geode logs for error messages

## Credits

- **Apple's GameController Framework**: For providing the MFi controller API
- **Geode SDK Team**: For the amazing modding framework
- **RobTop**: For creating Geometry Dash

## License

This mod is open source and follows the Geode SDK guidelines.

## Support

For issues, feature requests, or questions:
- Join the [Geode Discord](https://discord.gg/9e43WMKzhp)
- Open an issue on GitHub

## Changelog

### v1.0.0
- Initial release
- Full MFi controller support
- Automatic controller detection
- Basic button mappings for gameplay and menus
