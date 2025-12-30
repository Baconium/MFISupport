# Controller Button Textures

## About PC-Style Button Prompts

On PC, Geometry Dash shows controller button icons underneath menu items when a controller is connected. These textures help players know which buttons do what.

## Implementation Status

### Current Approach
The mod attempts to force Geometry Dash into "controller mode" by calling:
```cpp
mfisupport::onControllerConnectionChanged(true);
```

This should trigger the game to show button textures, but it depends on internal game flags that may not be accessible in GD 2.2074 iOS.

### Expected Button Textures

Based on PC version, you should see these button glyphs:

**Main Menu:**
- 🎮 **A** icon near "Play" button
- 🎮 **X** icon near "Icon Kit" button  
- 🎮 **Y** icon near "Creator" button
- 🎮 **B** icon near "Exit" button
- 🎮 **Start** icon near "Settings" button

**Pause Menu:**
- 🎮 **A** → Resume
- 🎮 **B** → Quit
- 🎮 **X** → Restart
- 🎮 **Y** → Practice

**Level Select:**
- 🎮 **A** → Play
- 🎮 **B** → Back
- 🎮 **D-pad** arrows for navigation

## Texture Locations

Controller button textures are typically found in the game's sprite sheets:
- `GJ_GameSheet.plist/.png` - Main UI elements
- `GJ_GameSheet02.plist/.png` - Additional UI elements
- `GJ_GameSheetEditor.plist/.png` - Editor UI

Common button sprite names (from PC version):
- `controllerBtn_A_001.png`
- `controllerBtn_B_001.png`
- `controllerBtn_X_001.png`
- `controllerBtn_Y_001.png`
- `controllerBtn_DpadUp_001.png`
- `controllerBtn_DpadDown_001.png`
- `controllerBtn_LB_001.png`
- `controllerBtn_RB_001.png`
- `controllerBtn_Start_001.png`

*(Note: Exact names may vary in iOS version)*

## How It Works on PC

1. Game detects controller via DirectInput/XInput
2. Sets internal `m_bControllerMode = true`
3. Button sprites appear under menu items
4. Sprites update based on controller type (Xbox/PS/Generic)

## iOS Implementation Challenge

**Problem**: GD 2.2074 iOS Geode bindings don't expose `GameManager::m_bControllerMode`

**Solutions**:
1. ✅ **Current**: Force controller mode at startup (may not work)
2. 🔄 **Fallback**: Show text hints instead of icons (already implemented)
3. 🔮 **Future**: Wait for Geode bindings to expose the flag
4. 🔮 **Future**: Patch the flag directly via memory access

## Text Hints Fallback

Since button textures may not appear, the mod displays text hints:
```cpp
auto* label = cocos2d::CCLabelBMFont::create(
    "A: Play  X: Icon Kit  Y: Creator  B: Exit  Start: Options", 
    "bigFont.fnt"
);
```

These are always visible when a controller is connected, ensuring players know the controls even without the PC-style button icons.

## Testing Button Texture Visibility

### Check 1: Connect Controller
- Pair your MFi controller
- Open GD with mod enabled
- Look for button icons under menu items

### Check 2: Check Logs
Look for this log message:
```
MFI: Controller UI mode FORCED -> true (always show button prompts)
```

### Check 3: Fallback Text
If icons don't appear, you should see text hints at the bottom of screens like:
- Main Menu: "A: Play X: Icon Kit Y: Creator..."
- Pause Menu: "A: Resume B: Quit X: Restart..."

## Future Improvements

### If Textures Don't Appear
1. **Request Geode Bindings**: Ask Geode team to expose `m_bControllerMode`
2. **Memory Patching**: Directly write to the flag's memory address
3. **Custom Sprites**: Add our own button sprite overlay system
4. **Hybrid Approach**: Mix text + custom icons

### Custom Icon System
Could implement our own button icon overlay:
```cpp
// Create custom button sprite from game textures
auto* buttonA = CCSprite::create("gj_button_01.png");
addChild(buttonA);
```

This would bypass the need for the internal controller flag entirely.

## Resource URLs

Controller button textures from GD 2.207 can be viewed at:
https://baconium.dev/gdresources/2.207/

Search the sprite sheets for controller-related sprites. They may be:
- Individual sprites: `controller_*.png`
- Inside sprite sheets: Check `.plist` files for frame names
- Custom icons: Game may generate them programmatically

## Summary

- ✅ Mod forces controller mode activation
- ✅ Text hints always show as fallback
- ⚠️ Button textures may not appear due to binding limitations
- 🔮 Future updates may enable full texture support

The text hints ensure the mod is fully functional even without PC-style button textures. Players can see all controls and the controller works perfectly; it's purely a visual enhancement that's missing.
