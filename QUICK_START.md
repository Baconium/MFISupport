# Quick Reference - MFI Controller Support

## To Build & Test

```bash
# On macOS only (cannot cross-compile iOS from Linux)
cd /path/to/MFISupport
geode build -p ios
# Output: build-ios/MFISupport.geode
```

## What Changed (Latest)

| Item | Before | After |
|------|--------|-------|
| Menu Input Method | `handleButton()` calls | `keyDown()` simulation |
| Button Display | "Controller Connected" text | "A: PLAY X: GALLERY ..." legend |
| Position | Top center | **Bottom center** |
| Font | bigFont | **goldFont** |

## Latest Code Updates

### MenuLayer Input (Lines 334-357)
```cpp
if (aPressed) this->keyDown(cocos2d::KEY_Return);      // A = Select
if (xPressed) this->keyDown(cocos2d::KEY_G);           // X = Gallery
if (yPressed) this->keyDown(cocos2d::KEY_C);           // Y = Creator
if (bPressed) this->keyDown(cocos2d::KEY_BackSpace);   // B = Back
if (menuPressed) this->keyDown(cocos2d::KEY_O);        // Start = Options
```

### Button Legend Display (Lines 405-428)
```cpp
auto* label = cocos2d::CCLabelBMFont::create(
    "A: PLAY  X: GALLERY  Y: CREATOR  B: EXIT  START: OPTIONS",
    "goldFont.fnt"
);
label->setPosition({vs.width * 0.5f, 10.f});  // Bottom center
```

## Expected Behavior

**On Menu Screen:**
- ✅ Button legend appears at bottom
- ✅ A button selects current item
- ✅ D-Pad navigates menu
- ✅ B button exits/returns
- ✅ Legend vanishes when disconnected

**In Gameplay:**
- ✅ Analog stick = movement
- ✅ Buttons = gameplay actions
- ✅ Pause menu = full controller support

## File Map

```
MFISupport/
├── src/
│   ├── main.cpp           ← Game integration (782 lines)
│   ├── MFIController.mm   ← iOS hardware layer
│   └── MFIController.h    ← C++ interface
├── CMakeLists.txt         ← Build config
├── mod.json               ← Mod metadata
└── docs/
    ├── BUILD_AND_TEST.md          ← Complete build guide
    ├── IMPLEMENTATION_DETAILS.md   ← Architecture
    ├── CHANGES_SUMMARY.md          ← This session's changes
    ├── CONTROLLER_FEATURES.md      ← Feature mapping
    └── BUTTON_LAYOUT.txt           ← ASCII reference
```

## Menu Button Mapping

| Button | Action |
|--------|--------|
| A | Select highlighted |
| B | Exit/Back |
| X | Gallery/Customize |
| Y | Creator/Online |
| Start | Options/Settings |
| D-Pad ↑↓ | Navigate up/down |
| D-Pad ← → | Navigate left/right |

## Gameplay Button Mapping

| Control | Action |
|---------|--------|
| Left Stick | Move left/right |
| A Button | Jump |
| B | Alternate jump |
| Start | Pause menu |

## Expected Log Messages

```
✓ MFI: Controller connected = true
✓ MFI: MenuLayer - A pressed
✓ MFI: MenuLayer button hints shown
✓ MFI: PlayLayer controller input processing
```

## Troubleshooting

| Issue | Quick Fix |
|-------|-----------|
| Menu doesn't respond | Check logs for button press messages |
| Button legend not showing | Try font "bigFont.fnt" instead of "goldFont.fnt" |
| Controller not detected | Ensure Bluetooth connection, restart GD |
| Crashes on connect | Check Geode logs for null pointer errors |
| No key responses | Verify KEY_Return, KEY_G etc. are valid codes |

## Build Requirement

⚠️ **IMPORTANT**: Cannot build on Linux for iOS
- Requires **macOS** for iOS compilation
- Linux cross-compilation not supported by Apple toolchain
- Use Mac Mini, Hackintosh, or cloud Mac service

## Testing Checklist

- [ ] Build succeeds on macOS
- [ ] .geode file created
- [ ] Installed to GD mods folder
- [ ] Controller connects and detected
- [ ] Button legend visible on menu
- [ ] Buttons navigate menu correctly
- [ ] Gameplay input works
- [ ] Disconnection handled gracefully

## Key Constants Used

```cpp
cocos2d::KEY_Return      // A button
cocos2d::KEY_BackSpace   // B button
cocos2d::KEY_G           // X button (Gallery)
cocos2d::KEY_C           // Y button (Creator)
cocos2d::KEY_O           // Start (Options)
cocos2d::KEY_ArrowUp     // D-Pad Up
cocos2d::KEY_ArrowDown   // D-Pad Down
```

## Font Options

**Primary**: `"goldFont.fnt"` - Fancy, matches GD aesthetic  
**Fallback**: `"bigFont.fnt"` - Standard, always available  
**Last Resort**: `"arial.fnt"` - System font  

## Position Coordinates

**Button Legend**:
- **X**: `vs.width * 0.5f` (center horizontally)
- **Y**: `10.f` (10 pixels from bottom)
- **Anchor**: `{0.5f, 0.f}` (bottom-center)
- **Scale**: `0.4f` (readable without blocking UI)

## Most Important Changes

1️⃣ **Input Method**: Switched to `keyDown()` for better menu integration  
2️⃣ **Button Display**: Now shows actual button legend instead of status text  
3️⃣ **Positioning**: Moved to bottom for better visibility  
4️⃣ **Edge Detection**: Prevents key repeat and multiple triggers  

## Next Actions

1. Transfer code to **macOS**
2. Run `geode build -p ios`
3. Copy `.geode` to iOS device
4. Install in Geometry Dash
5. Connect MFi controller
6. Test menu & gameplay
7. Check logs for issues

## Support Resources

- **Logs**: Geode log viewer in GD settings
- **Issues**: Check "Most Recent Changes" above
- **Contacts**: Community Discord (see mod.json)

---
**Status**: Ready for macOS build  
**Test On**: iOS 13+ with MFi controller  
**Last Update**: 2024 (Latest session)
