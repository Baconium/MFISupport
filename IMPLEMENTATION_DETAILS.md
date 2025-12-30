# MFI Controller Support - Implementation Summary

## What Was Implemented

This mod adds full MFi (Made for iPhone/iPad/iPod) controller support to Geometry Dash iOS version 2.2074, providing complete gamepad control throughout the game.

### Core Features:
1. **Hardware Detection**: Automatically detects connected MFi controllers
2. **Menu Navigation**: Full d-pad and button support for menus
3. **Gameplay Control**: Analog sticks and buttons for playing levels
4. **Pause Menu**: Controller support in pause screen
5. **Level Selection**: Navigate and select levels with controller
6. **Creator Menu**: Access online features with controller
7. **Visual Feedback**: On-screen button legend shows available controls

## Code Architecture

### Three-Layer Implementation:

#### 1. Hardware Layer (`src/MFIController.mm`)
- **Language**: Objective-C++
- **Framework**: Apple GameController
- **Purpose**: Low-level iOS controller API
- **Features**:
  - Button press detection (A, B, X, Y, LB, RB, LT, RT, Menu)
  - D-pad input handling
  - Left/Right analog stick input
  - Connection/disconnection notifications
  - Modern iOS 13+ API usage (no deprecated calls)

#### 2. C++ Interface (`src/MFIController.h`)
- **Purpose**: Bridge between Objective-C++ hardware and C++ game logic
- **Provides**:
  - `ControllerState` struct (all buttons and analog positions)
  - `MFIControllerManager` singleton class
  - Connection status queries
  - State reading functions

#### 3. Game Integration (`src/main.cpp`)
- **Language**: C++ with Geode SDK
- **Purpose**: Hook into game layers and route controller input
- **Layers Modified**:
  - `PlayLayer` - Gameplay input handling
  - `MenuLayer` - Main menu navigation
  - `PauseLayer` - Pause menu control
  - `LevelSelectLayer` - Level selection
  - `CreatorLayer` - Online menu navigation
- **Input Method**: Keyboard event simulation using `keyDown()`

## How It Works

### 1. Connection Detection
```
MFi Controller → GameController API → MFIController.mm
                                          ↓
                                    MFIControllerManager
                                          ↓
                                    C++ game hooks
```

### 2. Input Processing
- **State Tracking**: Each button/axis stored in `ControllerState`
- **Edge Detection**: Only trigger action on button press (not hold)
- **Deadzone Handling**: 20% deadzone for analog sticks
- **Key Simulation**: Convert button presses to keyboard events

### 3. Menu Input Routing
```
A Button (pressed)
  ↓
MenuLayer::update() detects via MFIControllerManager::getState()
  ↓
Calls this->keyDown(KEY_Return)  // Simulates Enter key
  ↓
Game menu handles Return key
  ↓
Menu action executes (Play, Gallery, etc.)
```

## Button Mapping Reference

### Main Menu
| Button | Action |
|--------|--------|
| A | Select highlighted item |
| X | Open Gallery/Icon Kit |
| Y | Open Creator/Online menu |
| B | Exit/Go Back |
| Start | Open Options/Settings |
| D-Pad Up | Navigate up |
| D-Pad Down | Navigate down |
| D-Pad Left | Previous category |
| D-Pad Right | Next category |

### Gameplay
| Input | Action |
|-------|--------|
| Left Analog Stick | Move player left/right |
| Right Analog Stick | (Reserved for future use) |
| A Button | Jump (classic mode) / Platformer controls |
| B Button | (Alternate jump / special ability) |
| X Button | (Special move / ability) |
| Y Button | (Special move / ability) |
| LB/RB | (Dash / special action) |
| Start | Open pause menu |

### Pause Menu
| Button | Action |
|--------|--------|
| A | Select option |
| B | Resume game |
| D-Pad | Navigate options |
| Start | Resume game |

## Key Implementation Details

### State Tracking with Edge Detection
```cpp
// Detect button press (transition from not pressed to pressed)
bool aPressed = aPressed && !m_prevA;  // Was released, now pressed
if (aPressed) {
    // Handle button press once
    this->keyDown(cocos2d::KEY_Return);
}
m_prevA = aPressed;  // Update state for next frame
```

### Analog Stick Handling
```cpp
float leftX = state.leftStickX;    // -1.0 to 1.0
float leftY = state.leftStickY;    // -1.0 to 1.0

// Apply deadzone (ignore small movements)
if (fabs(leftX) < 0.2f) leftX = 0;  // 20% deadzone
if (fabs(leftY) < 0.2f) leftY = 0;

// Use for movement control
float moveDir = (leftX > 0) ? 1 : (leftX < 0) ? -1 : 0;
```

### On-Screen Button Legend
```
"A: PLAY  X: GALLERY  Y: CREATOR  B: EXIT  START: OPTIONS"
```
- Displayed at bottom of menu screen
- Hidden when controller disconnects
- Updates automatically when reconnecting

## Build Instructions

### On macOS:
```bash
# Navigate to project directory
cd /path/to/MFISupport

# Build for iOS
geode build -p ios

# Output file location
build-ios/MFISupport.geode
```

### Deploy:
1. Transfer `.geode` file to iOS device
2. Open in Geometry Dash (mod auto-installs)
3. Enable in mod settings
4. Restart Geometry Dash
5. Connect MFi controller via Bluetooth

## Testing Sequence

### 1. Verify Controller Detection
- [ ] Launch Geometry Dash
- [ ] Connect MFi controller via Bluetooth
- [ ] Check Geode logs for "Controller connected = true"
- [ ] Button legend should appear on menu

### 2. Test Menu Navigation
- [ ] Use D-Pad to move between menu items
- [ ] Press A to select items
- [ ] Navigate all menu sections
- [ ] Return to main menu with B button

### 3. Test Gameplay
- [ ] Select a level
- [ ] Verify analog stick controls character
- [ ] Test jump button works
- [ ] Test pause menu access

### 4. Test Disconnection
- [ ] Turn off/disconnect controller
- [ ] Verify "Controller disconnected" in logs
- [ ] Button legend should disappear
- [ ] Game should still be playable with touch

## Logging & Debugging

### Enable Geode Log Viewer:
1. Open Geometry Dash settings
2. Go to Mods section
3. Select MFI Support
4. Enable "Show Console"
5. Connect controller and observe logs

### Key Log Messages to Expect:
```
[INFO] MFI: Controller connected = true
[INFO] MFI: MenuLayer - A pressed
[INFO] MFI: MenuLayer button hints shown
[INFO] MFI: PlayLayer controller input processing...
```

## Known Limitations

1. **Font Resources**: Uses standard Cocos2d fonts (might vary by GD version)
2. **Keyboard Simulation**: Some menu items might not respond to keyboard simulation
3. **Game Prompts**: Uses custom button labels instead of game's native prompts
4. **Vibration**: Not implemented yet (future enhancement)
5. **Analog Sensitivity**: Fixed deadzone (20%) - no customization yet

## Future Enhancements

1. **Custom Settings Page**: Adjust deadzone and sensitivity
2. **Vibration Support**: Haptic feedback on button presses
3. **Native Button Prompts**: Integrate with GD's controller UI
4. **Button Remapping**: Let users customize button functions
5. **Turbo Mode**: Configurable button rapid-fire
6. **Save Controller Profiles**: Store per-game settings

## Troubleshooting Guide

### Problem: "Controller connected" but buttons don't work
**Solutions**:
- Check if keyboard shortcuts are enabled in GD settings
- Try different MFi controller
- Restart Geometry Dash
- Update iOS to latest version

### Problem: Button legend shows but menus don't respond
**Cause**: Keyboard simulation not working with specific menus
**Solution**: 
- Check logs for button press messages
- Verify `keyDown()` calls are reaching the right key codes
- Try alternative key mappings

### Problem: Crashes when connecting controller
**Cause**: Null pointer in state access
**Check**:
- Verify MFIControllerManager::getState() is returning valid data
- Ensure all pointer dereferences have null checks
- Check for memory leaks in Objective-C++ code

### Problem: Input lag or delayed response
**Cause**: Too much game load or scheduling conflicts
**Solutions**:
- Check game performance (fps might drop)
- Reduce update frequency if needed
- Profile with Instruments on macOS

## Support & Contributions

- **Issues**: Report with detailed logs from Geode
- **Suggestions**: Include video of expected vs actual behavior
- **Code**: Include detailed commit messages explaining changes

---

**Version**: 1.0.0  
**Target Game**: Geometry Dash iOS 2.2074  
**Target Framework**: Geode 4.10.0  
**iOS Minimum**: iOS 13.0 (for modern GameController API)  
**Last Updated**: 2024
