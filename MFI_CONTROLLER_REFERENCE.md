# MFi Controller Reference

## What are MFi Controllers?

MFi (Made for iPhone/iPad/iPod) controllers are game controllers that are officially certified by Apple to work with iOS, iPadOS, macOS, and tvOS devices. They use Apple's standardized GameController framework for seamless integration.

## Certification

MFi controllers must:
- Meet Apple's hardware specifications
- Pass Apple's certification process
- Support standard button layouts
- Work with Apple's GameController framework

## Popular MFi Controllers

### Console Controllers
- **Xbox Wireless Controller** (Series X/S, One)
- **PlayStation DualShock 4** (PS4)
- **PlayStation DualSense** (PS5)

### Dedicated Mobile Controllers
- **Backbone One** - iPhone attachment controller
- **Razer Kishi** - Universal mobile controller
- **SteelSeries Nimbus+** - Wireless controller
- **SteelSeries Stratus+** - Wireless controller
- **Gamevice** - Attachment controller
- **Rotor Riot** - Wired iOS controller

### Premium Options
- **8BitDo** controllers (various models)
- **PowerA MOGA** controllers
- **Mad Catz C.T.R.L.** controllers

## Controller Profiles

Apple defines standard profiles that all MFi controllers must support:

### 1. Micro Gamepad Profile
**Minimal controls for casual games:**
- D-pad
- 2 buttons (A, X)
- Optional Menu button

### 2. Standard Gamepad Profile
**Basic gaming controls:**
- D-pad
- 4 face buttons (A, B, X, Y)
- 2 shoulder buttons
- Optional Menu button

### 3. Extended Gamepad Profile (Used by this mod)
**Full-featured gaming:**
- D-pad
- 4 face buttons (A, B, X, Y)
- 2 shoulder buttons (L/R)
- 2 triggers (LT/RT)
- 2 thumbsticks with clickable buttons
- Menu and Options buttons
- Optional: Home button

### 4. Motion Profile
**Additional motion sensors:**
- Gyroscope
- Accelerometer
- Gravity vector
- Rotation rate

## GameController Framework Features

### Input Methods

**Polling:**
```objective-c
// Read current button state
BOOL isPressed = gamepad.buttonA.pressed;
float value = gamepad.leftTrigger.value;
```

**Callbacks (Used by this mod):**
```objective-c
// Register for button press events
gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
    // Handle button press
};
```

### Controller Discovery

**Automatic Detection:**
- Controllers are automatically discovered when connected
- Notifications sent on connect/disconnect
- Support for wireless and wired connections

### Pairing

**Bluetooth Pairing:**
1. Put controller in pairing mode (varies by controller)
2. Go to iOS Settings > Bluetooth
3. Select controller from available devices
4. Pair and connect

**Direct Connection:**
- Some controllers (like Backbone One) connect via Lightning port
- Instant connection without pairing needed

## Controller Button Layout

### Standard Face Buttons
```
        Y
        |
    X - + - B
        |
        A
```

### Shoulder Buttons
```
Left Shoulder (L)      Right Shoulder (R)
Left Trigger (LT)      Right Trigger (RT)
```

### Thumbsticks
```
Left Thumbstick (L3)   Right Thumbstick (R3)
- X axis: -1 to 1     - X axis: -1 to 1
- Y axis: -1 to 1     - Y axis: -1 to 1
- Click: L3 button    - Click: R3 button
```

### D-Pad
```
    Up
    |
Left-+-Right
    |
   Down
```

## iOS Requirements

- **iOS 7.0+**: Basic MFi controller support
- **iOS 13.0+**: Xbox and PlayStation controller support
- **iOS 14.0+**: Enhanced controller features
- **iOS 15.0+**: Additional controller profiles

## Battery Life

Most wireless MFi controllers offer:
- **20-40 hours** of gameplay on a full charge
- **Rechargeable** via USB-C or Lightning
- **Battery indicators** available via GameController framework

## Latency

MFi controllers provide:
- **Bluetooth LE**: ~10-20ms typical latency
- **Wired**: <5ms latency
- **Optimized for gaming**: Low-latency mode on supported devices

## Compatibility Matrix

| Controller Type | iPhone | iPad | Apple TV | Mac |
|----------------|--------|------|----------|-----|
| Xbox Wireless | ✅ | ✅ | ✅ | ✅ |
| PlayStation | ✅ | ✅ | ✅ | ✅ |
| MFi Certified | ✅ | ✅ | ✅ | ✅ |
| Generic Bluetooth | ❌ | ❌ | ❌ | ⚠️ |

## Technical Specifications

### Communication Protocol
- **Bluetooth 4.0+** (BLE)
- **Lightning connector** (wired controllers)
- **USB-C** (newer iPads)

### Input Precision
- **Buttons**: Digital (pressed/released)
- **Triggers**: Analog (0.0 to 1.0)
- **Thumbsticks**: Analog (-1.0 to 1.0 on X/Y)
- **D-Pad**: Digital (4 directions)

### Polling Rate
- **Typical**: 60 Hz (60 updates per second)
- **High-Performance**: Up to 120 Hz on supported controllers

## GameController Framework API

### Key Classes

**GCController**
- Represents a physical or virtual controller
- Properties: vendorName, playerIndex, battery, etc.

**GCExtendedGamepad**
- Extended gamepad profile
- Access to all buttons, triggers, thumbsticks

**GCControllerButtonInput**
- Individual button state
- Properties: value, pressed
- Callbacks: valueChangedHandler

**GCControllerDirectionPad**
- D-pad or thumbstick
- Properties: xAxis, yAxis
- Individual button access (up, down, left, right)

### Notifications

**GCControllerDidConnectNotification**
- Sent when controller connects
- Object contains GCController instance

**GCControllerDidDisconnectNotification**
- Sent when controller disconnects
- Object contains disconnected GCController

## Best Practices

### Input Handling
1. ✅ Use valueChangedHandler for real-time input
2. ✅ Check controller connection state before processing
3. ✅ Handle controller disconnection gracefully
4. ✅ Support multiple input methods (touch + controller)

### User Experience
1. ✅ Show visual feedback for controller actions
2. ✅ Display button prompts that match connected controller
3. ✅ Allow controller remapping
4. ✅ Pause game when controller disconnects

### Performance
1. ✅ Process input on main thread or dedicated input thread
2. ✅ Minimize allocations in input callbacks
3. ✅ Use appropriate dead zones for thumbsticks
4. ✅ Batch similar input events when possible

## Common Issues & Solutions

### Controller Not Recognized
**Problem**: Controller connects but isn't detected by game
**Solution**: 
- Restart the app
- Unpair and re-pair controller
- Update controller firmware
- Check if controller is MFi-certified

### Input Lag
**Problem**: Noticeable delay between button press and action
**Solution**:
- Use Bluetooth over WiFi channel separation
- Update iOS to latest version
- Reduce other Bluetooth devices nearby
- Use wired connection if available

### Buttons Not Working
**Problem**: Some buttons don't register
**Solution**:
- Check if game supports extended gamepad profile
- Verify button mappings
- Test controller in another app
- Calibrate controller (if supported)

### Battery Draining Fast
**Problem**: Controller battery depletes quickly
**Solution**:
- Disable controller auto-sleep
- Update controller firmware
- Check for background apps using controller
- Use wired connection for long sessions

## Resources

- **Apple GameController Documentation**: https://developer.apple.com/documentation/gamecontroller
- **MFi Program**: https://mfi.apple.com/
- **Controller Testing**: Use Apple's "GameController Tester" app
- **Controller Compatibility List**: https://support.apple.com/en-us/HT210414

## Controller-Specific Pairing Instructions

### Xbox Controller
1. Press and hold Xbox button to turn on
2. Press and hold pairing button (top of controller) until Xbox button flashes
3. Connect via Bluetooth settings

### PlayStation DualShock 4 / DualSense
1. Press and hold PlayStation + Share buttons (DualShock 4)
2. Press and hold PlayStation + Create buttons (DualSense)
3. Light bar will flash, connect via Bluetooth settings

### Backbone One
1. Connect directly to iPhone Lightning port
2. Automatic connection, no pairing needed

### SteelSeries Nimbus
1. Hold power button for 3 seconds
2. LED will flash, connect via Bluetooth settings

---

This reference guide covers the essential information about MFi controllers and their integration with iOS. The mod implementation uses these principles to provide native controller support for Geometry Dash.
