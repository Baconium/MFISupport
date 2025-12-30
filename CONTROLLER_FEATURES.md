# MFI Controller Support - Complete Feature List

This mod adds comprehensive MFI (Made for iPhone) controller support to Geometry Dash on iOS, bringing feature parity with the PC version's controller support.

## ✅ Implemented Features

### Main Menu
- **A Button**: Start / Play (Main Level Select)
- **X Button**: Open Icon Kit (Character Customization)  
- **Y Button**: Open Creator Menu (Online/Editor)
- **B Button**: Exit Game (with confirmation prompt)
- **D-Pad / Left Stick**: Navigate main menu buttons
- **Start/Menu Button**: Open Options / Settings

### Level Select
- **A Button**: Start selected level
- **B Button**: Back to Main Menu
- **D-Pad / Left Stick**: Scroll through levels (Stereo Madness, Back on Track, etc.)
- **Y Button**: Toggle Practice Mode

### Classic Gameplay (Auto-scrolling levels)
- **A Button**: Jump / Action
- **RT (Right Trigger)**: Jump / Action (Alternative)
- **LT (Left Trigger)**: Jump / Action (Alternative)
- **Start/Menu Button**: Pause Game
- **X Button**: Restart level (in practice mode)
- **Y Button**: Remove Practice Checkpoint (Manual)

### Platformer Gameplay
- **Left Stick / D-Pad Left/Right**: Move Left / Right
- **A Button**: Jump
- **Start/Menu Button**: Pause Game
- **X Button**: Restart level (in practice mode)
- **Y Button**: Remove Practice Checkpoint (Manual)

### Pause Menu
- **A Button**: Resume Level
- **B Button**: Exit to Level Select
- **X Button**: Restart Level
- **Y Button**: Toggle Practice Mode
- **LB (Left Bumper)**: Previous Song (Practice Mode) *[Planned]*
- **RB (Right Bumper)**: Next Song (Practice Mode) *[Planned]*
- **D-Pad Up/Down**: Navigate menu options

### Creator Menu
- **A Button**: Select / Open current menu item
- **B Button**: Back to main menu
- **X Button**: Open 'Create' (Level Editor list / My Levels)
- **Y Button**: Open 'Search' levels (Online Levels)
- **D-Pad / Left Stick**: Navigate menu buttons (Daily, Weekly, Gauntlets, etc.)

## ⏳ Not Yet Implemented

### Level Editor
- Cursor movement and camera panning
- Object placement and selection
- Layer changes
- Object editing
These features are complex and require cursor emulation, which is being deferred for now.

## 🎮 Controller Button Texture Display

The mod attempts to force Geometry Dash into "controller mode" so that controller button textures (like those shown on PC) are always visible underneath menu items. This helps players know what each button does.

**Note**: Since GD 2.2074 iOS bindings don't expose the `m_bControllerMode` flag directly, button textures may only appear when the game itself detects controller input or has native controller support enabled.

## 🔧 Technical Details

### Controller State Tracking
- All button states are tracked with edge detection (press/release)
- Analog stick deadzone: 0.2 (20%)
- Left stick is used for platformer movement with proper state management
- D-pad provides backup input for all navigation

### Input Method
- Uses native iOS GameController framework via MFI (Made for iPhone) protocol
- Supports extended gamepad profile (standard modern controllers)
- Button mappings follow Xbox-style conventions (A/B/X/Y)

### Supported Controllers
Any MFi-certified controller should work, including:
- Xbox Wireless Controller (Bluetooth)
- PlayStation DualShock 4 / DualSense
- SteelSeries Nimbus+
- Backbone One
- Razer Kishi
- Any other MFi-certified gamepad

## 📝 Notes

### Practice Mode
- Manual checkpoint placement would typically use the "Back/Select" button but isn't implemented yet
- Y button removes the last checkpoint
- Practice mode toggle works from pause menu and level select

### Platformer Mode Detection
The mod detects platformer mode levels and automatically switches input handling to support left stick / D-pad horizontal movement.

### Always-Visible Prompts
The mod is designed to force controller button prompts to always be visible (even without a controller connected), matching PC behavior. However, this depends on accessing internal game flags that may not be exposed in the current GD version.

## 🐛 Known Limitations

1. Button textures may not show up unless:
   - A controller is physically connected
   - The game has internal controller mode support
   - Future Geode bindings expose the controller flag

2. Song navigation in practice mode pause menu (LB/RB) is not yet implemented

3. Editor functionality is not implemented (requires cursor emulation)

## 🔮 Future Improvements

- Haptic feedback for jumps and collisions
- Custom button mapping configuration
- Sensitivity settings for analog sticks
- Song navigation in practice pause menu
- Editor support with cursor emulation
