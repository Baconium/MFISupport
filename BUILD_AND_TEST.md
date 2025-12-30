# Build and Test Instructions for MFI Controller Support

## Important Note
This project requires macOS for compilation, as iOS development cannot be cross-compiled on Linux. The code changes below have been made and are ready to test on macOS.

## Recent Code Changes (Latest Update)

### 1. MenuLayer Button Input - Updated for Better Integration
**File**: `src/main.cpp` (Lines 334-357)

Changed from `handleButton()` simulation to proper key press simulation using `keyDown()`:

```cpp
// A button - Start/Play (Main Level Select)
if (aPressed) {
    log::info("MFI: MenuLayer - A pressed");
    this->keyDown(cocos2d::KEY_Return);  // Enter key
}

// X button - Open Icon Kit (Character Customization)  
if (xPressed) {
    log::info("MFI: MenuLayer - X pressed");
    this->keyDown(cocos2d::KEY_G);  // Gallery shortcut
}

// Y button - Open Creator Menu (Online/Editor)
if (yPressed) {
    log::info("MFI: MenuLayer - Y pressed");
    this->keyDown(cocos2d::KEY_C);  // Creator shortcut
}

// B button - Exit Game (with confirmation)
if (bPressed) {
    log::info("MFI: MenuLayer - B pressed");
    this->keyDown(cocos2d::KEY_BackSpace);  // Back/Exit
}

// Start/Menu button - Open Options/Settings
if (menuPressed) {
    log::info("MFI: MenuLayer - Start pressed");
    this->keyDown(cocos2d::KEY_O);  // Options shortcut
}
```

### 2. Button Labels Display - Now Shows Controller Hints
**File**: `src/main.cpp` (Lines 405-428)

Updated to display button hints similar to GD 2.1 screenshot:

```cpp
void updateHint() {
    bool connected = MFIControllerManager::isControllerConnected();
    if (connected) {
        if (!m_fields->m_controllerHint) {
            // Show button legend at bottom of screen
            auto* label = cocos2d::CCLabelBMFont::create(
                "A: PLAY  X: GALLERY  Y: CREATOR  B: EXIT  START: OPTIONS", 
                "goldFont.fnt"
            );
            if (label) {
                label->setScale(0.4f);
                auto vs = cocos2d::CCDirector::sharedDirector()->getWinSize();
                label->setAnchorPoint({0.5f, 0.f});
                label->setPosition({vs.width * 0.5f, 10.f});
                label->setColor({255, 255, 255});
                this->addChild(label, 9999);
                m_fields->m_controllerHint = label;
                log::info("MFI: MenuLayer button hints shown");
            }
        }
    } else {
        if (m_fields->m_controllerHint) {
            m_fields->m_controllerHint->removeFromParentAndCleanup(true);
            m_fields->m_controllerHint = nullptr;
        }
    }
}
```

## How to Build on macOS

1. **Install Geode SDK** (if not already installed):
   ```bash
   brew install geode-sdk
   ```

2. **Navigate to project directory**:
   ```bash
   cd /path/to/MFISupport
   ```

3. **Build for iOS**:
   ```bash
   geode build -p ios
   ```

4. **Build output** will be at:
   ```
   build-ios/MFISupport.geode
   ```

## Testing Checklist

### Before Testing:
- [ ] Build successfully completes on macOS
- [ ] MFISupport.geode file created
- [ ] File installed to Geometry Dash mods folder

### Controller Connection:
- [ ] Controller connects and is detected by the system
- [ ] Green "Controller Connected" text appears in logs (check Geode log viewer)
- [ ] Controller buttons respond to presses (check logs for button press messages)

### Menu Testing:
- [ ] **A Button**: Should press currently selected menu item (Play, Gallery, Creator, etc.)
- [ ] **X Button**: Should trigger Gallery/Icon Kit
- [ ] **Y Button**: Should trigger Creator/Online menu
- [ ] **B Button**: Should exit or go back
- [ ] **Start Button**: Should open Options/Settings
- [ ] **D-Pad Up/Down**: Should navigate menu items
- [ ] **D-Pad Left/Right**: Should navigate between menu sections

### Visual Display:
- [ ] Button legend appears at bottom of menu screen
- [ ] Text reads: "A: PLAY  X: GALLERY  Y: CREATOR  B: EXIT  START: OPTIONS"
- [ ] Text disappears when controller disconnects
- [ ] Text appears when controller reconnects

### Gameplay Testing:
- [ ] Analog sticks control player movement (classic mode)
- [ ] Button inputs work in gameplay
- [ ] Pause menu responds to controller
- [ ] Level selection responds to controller

## Expected Results

When you connect an MFi controller to your iOS device:

1. **Immediate**: "Controller Connected" message appears briefly in logs
2. **Menu Screen**: 
   - Button hints text appears at the bottom
   - Controller buttons navigate and select menu items
   - No button prompts from the game's native system (this is expected - we're using keyboard simulation)
3. **Gameplay**: 
   - Controller fully controls the game as if using keyboard
   - All button combinations work as designed

## Troubleshooting

### Controls Don't Work
- Check Geode log viewer for button press messages
- Verify controller is properly connected (System Settings > Bluetooth)
- Try re-plugging the controller
- Check that all `keyDown()` calls in the code are using valid Cocos2d key constants

### Button Hints Don't Show
- Verify the font file "goldFont.fnt" exists in the game resources
- Check that the label creation code is being reached (look for log messages)
- Try changing font to "bigFont.fnt" if "goldFont.fnt" doesn't exist

### Game Crashes
- Check Geode log for crash messages
- Ensure all method calls (`keyDown()`, `addChild()`) are available in the hooked layer
- Verify no nullptr dereferences in the controller state structure

## Future Improvements

1. **Native Button Prompts**: Currently using keyboard simulation. Could integrate with GD's native controller detection to show game button prompts.
2. **Analog Stick Sensitivity**: Add settings for deadzone and sensitivity adjustments.
3. **Vibration Support**: Add haptic feedback when buttons are pressed.
4. **Pause Menu**: Enhanced pause menu support with full button mapping.
5. **Tutorial Mode**: In-game tutorial showing controller layout to new players.

## Technical Notes

- **Input Method**: Using `keyDown()` instead of direct button callbacks for better menu integration
- **Button Mapping**: D-pad and buttons mapped to familiar keyboard shortcuts
- **State Tracking**: Edge detection prevents key repeat issues
- **Font**: Using standard Cocos2d fonts from the game resources
- **Layer Hooks**: Modifying MenuLayer, PlayLayer, PauseLayer, LevelSelectLayer, CreatorLayer

## Contact & Support

If you encounter issues:
1. Check the logs in Geode log viewer
2. Verify your controller is MFi certified
3. Try different MFi controllers if available
4. Report issues with detailed logs from Geode

---
**Last Updated**: 2024 (After button integration improvements)
**Status**: Ready for macOS build and testing
