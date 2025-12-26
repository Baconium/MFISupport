# Changelog

## v1.0.0 (2025-12-26)

### Initial Release 🎉

- Added full MFi controller support for iOS
- Automatic controller detection and connection
- Hot-plugging support (connect/disconnect anytime)
- Default button mappings for gameplay:
  - A button, Right Trigger, and D-pad Up for jumping
  - B button and Left Trigger for secondary actions
- Controller-friendly menu navigation
- Support for all MFi-certified controllers
- Integration with Apple's GameController framework
- Logging for controller connection/disconnection events

### Technical Implementation
- Hooks into `MenuLayer` for initialization
- Hooks into `PlayLayer` for gameplay input processing
- Hooks into `PauseLayer` for pause menu navigation
- Uses GCExtendedGamepad profile for maximum compatibility
- Properly linked with GameController and Foundation frameworks

### Known Limitations
- Currently uses fixed button mappings (customization planned for future)
- Menu navigation could be enhanced (planned for future updates)

---

**Note**: This is the first public release. Please report any issues or suggestions!
