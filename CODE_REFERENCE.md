# Code Reference - Key Implementations

## Button Input Handling (MenuLayer)

### Location: src/main.cpp, lines 334-378

#### Button Press Detection
```cpp
// Get current controller state
const auto& state = MFIControllerManager::getState();

// Detect button presses (false → true transitions)
bool aPressed = state.buttonA && !m_fields->m_prevA;
bool bPressed = state.buttonB && !m_fields->m_prevB;
bool xPressed = state.buttonX && !m_fields->m_prevX;
bool yPressed = state.buttonY && !m_fields->m_prevY;
bool lbPressed = state.buttonLB && !m_fields->m_prevLB;
bool rbPressed = state.buttonRB && !m_fields->m_prevRB;
bool menuPressed = state.buttonMenu && !m_fields->m_prevMenu;

// Update state for next frame (edge detection)
m_fields->m_prevA = state.buttonA;
m_fields->m_prevB = state.buttonB;
m_fields->m_prevX = state.buttonX;
m_fields->m_prevY = state.buttonY;
m_fields->m_prevLB = state.buttonLB;
m_fields->m_prevRB = state.buttonRB;
m_fields->m_prevMenu = state.buttonMenu;
```

#### Button Action Execution
```cpp
// A button - Start/Play (Main Level Select)
if (aPressed) {
    log::info("MFI: MenuLayer - A pressed");
    this->keyDown(cocos2d::KEY_Return);
}

// X button - Open Icon Kit (Character Customization)  
if (xPressed) {
    log::info("MFI: MenuLayer - X pressed");
    this->keyDown(cocos2d::KEY_G);
}

// Y button - Open Creator Menu (Online/Editor)
if (yPressed) {
    log::info("MFI: MenuLayer - Y pressed");
    this->keyDown(cocos2d::KEY_C);
}

// B button - Exit Game (with confirmation)
if (bPressed) {
    log::info("MFI: MenuLayer - B pressed");
    this->keyDown(cocos2d::KEY_BackSpace);
}

// Start/Menu button - Open Options/Settings
if (menuPressed) {
    log::info("MFI: MenuLayer - Start pressed");
    this->keyDown(cocos2d::KEY_O);
}
```

#### D-Pad Navigation
```cpp
// D-pad navigation - simulate arrow keys for menu navigation
if (upPressed) {
    this->keyDown(cocos2d::KEY_ArrowUp);
    m_fields->m_prevUp = true;
} else if (!upNow) {
    m_fields->m_prevUp = false;
}

if (downPressed) {
    this->keyDown(cocos2d::KEY_ArrowDown);
    m_fields->m_prevDown = true;
} else if (!downNow) {
    m_fields->m_prevDown = false;
}

if (leftPressed) {
    this->keyDown(cocos2d::KEY_ArrowLeft);
    m_fields->m_prevLeft = true;
} else if (!leftNow) {
    m_fields->m_prevLeft = false;
}

if (rightPressed) {
    this->keyDown(cocos2d::KEY_ArrowRight);
    m_fields->m_prevRight = true;
} else if (!rightNow) {
    m_fields->m_prevRight = false;
}
```

## Button Legend Display

### Location: src/main.cpp, lines 405-428

#### Full Implementation
```cpp
void updateHint() {
    bool connected = MFIControllerManager::isControllerConnected();
    if (connected) {
        if (!m_fields->m_controllerHint) {
            // Show button legend at bottom of screen: 
            // "A: PLAY  X: ICON KIT  Y: CREATOR  B: EXIT  START: OPTIONS"
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
            log::info("MFI: MenuLayer hints hidden");
        }
    }
}
```

#### Key Parameters Explained
```cpp
// Font selection (in order of preference)
"goldFont.fnt"      // Primary - matches GD aesthetic
"bigFont.fnt"       // Fallback - standard GD font
"arial.fnt"         // Last resort - system font

// Scale 0.4f breakdown
// - 0.3f = smaller, harder to read
// - 0.4f = readable, doesn't obstruct gameplay (current)
// - 0.5f = larger, might obstruct UI

// Position calculation
vs.width * 0.5f     // Horizontal center
10.f                // 10 pixels from bottom edge

// Anchor point {0.5f, 0.f}
// X: 0.5f = center horizontally
// Y: 0.f = position relative to bottom

// Z-order 9999
// Ensures legend appears above all other UI elements
```

## MenuLayer State Fields

### Location: src/main.cpp, class definition

```cpp
struct Fields {
    // Button state tracking
    bool m_prevA = false;
    bool m_prevB = false;
    bool m_prevX = false;
    bool m_prevY = false;
    bool m_prevLB = false;
    bool m_prevRB = false;
    bool m_prevLT = false;
    bool m_prevRT = false;
    bool m_prevMenu = false;
    
    // D-Pad state tracking
    bool m_prevUp = false;
    bool m_prevDown = false;
    bool m_prevLeft = false;
    bool m_prevRight = false;
    
    // Analog sticks
    float m_leftStickX = 0.0f;
    float m_leftStickY = 0.0f;
    
    // UI elements
    cocos2d::CCLabelBMFont* m_controllerHint = nullptr;
};
```

## PlayLayer Input Example

### Location: src/main.cpp (around line 140-160)

```cpp
// In PlayLayer::update()
bool connected = MFIControllerManager::isControllerConnected();
if (connected) {
    const auto& state = MFIControllerManager::getState();
    
    // Get analog stick values (-1.0 to 1.0)
    float leftX = state.leftStickX;
    float leftY = state.leftStickY;
    
    // Apply deadzone (ignore small movements)
    if (fabs(leftX) < 0.2f) leftX = 0;
    if (fabs(leftY) < 0.2f) leftY = 0;
    
    // Determine direction
    float moveDir = (leftX > 0) ? 1 : (leftX < 0) ? -1 : 0;
    
    // Send keyboard input for movement
    if (moveDir > 0) {
        this->keyDown(cocos2d::KEY_Right);
    } else if (moveDir < 0) {
        this->keyDown(cocos2d::KEY_Left);
    }
    
    // Handle jump button
    bool jumpPressed = state.buttonA && !m_fields->m_prevA;
    if (jumpPressed) {
        // Trigger jump action
        this->keyDown(cocos2d::KEY_Space);
    }
    m_fields->m_prevA = state.buttonA;
}
```

## ControllerState Structure

### Location: src/MFIController.h

```cpp
struct ControllerState {
    // Buttons
    bool buttonA = false;       // Green button (bottom)
    bool buttonB = false;       // Red button (right)
    bool buttonX = false;       // Blue button (left)
    bool buttonY = false;       // Yellow button (top)
    
    // Shoulders
    bool buttonLB = false;      // Left bumper
    bool buttonRB = false;      // Right bumper
    bool buttonLT = false;      // Left trigger (digital)
    bool buttonRT = false;      // Right trigger (digital)
    
    // Center buttons
    bool buttonMenu = false;    // Menu/Start button
    
    // D-Pad
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;
    
    // Analog sticks (-1.0 to 1.0)
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;
    
    // Triggers (0.0 to 1.0)
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;
};
```

## Edge Detection Pattern

### How Button Press Detection Works

```cpp
// Frame 0: User presses A button
state.buttonA = true
m_prevA = false (from initialization)
aPressed = true && !false = true  ✓ DETECTED
m_prevA = true  // Store for next frame

// Frame 1: User still holding A
state.buttonA = true
m_prevA = true (from previous frame)
aPressed = true && !true = false  ✗ NOT DETECTED
m_prevA = true

// Frame 2: User releases A
state.buttonA = false
m_prevA = true (still from press)
aPressed = false && !true = false  ✗ NOT DETECTED
m_prevA = false

// Frame 3: User presses A again
state.buttonA = true
m_prevA = false (just updated)
aPressed = true && !false = true  ✓ DETECTED
m_prevA = true
```

This prevents the same button press from triggering multiple times.

## Key Code Constants (Cocos2d)

```cpp
// Main buttons
cocos2d::KEY_Space          // A/Jump
cocos2d::KEY_Return         // A/Select
cocos2d::KEY_BackSpace      // B/Back
cocos2d::KEY_Escape         // B/Exit

// D-Pad / Navigation
cocos2d::KEY_ArrowUp        // D-Pad Up
cocos2d::KEY_ArrowDown      // D-Pad Down
cocos2d::KEY_ArrowLeft      // D-Pad Left
cocos2d::KEY_ArrowRight     // D-Pad Right

// Shortcuts
cocos2d::KEY_G              // Gallery
cocos2d::KEY_C              // Creator
cocos2d::KEY_O              // Options
cocos2d::KEY_P              // Pause
cocos2d::KEY_L              // Level Select

// Other
cocos2d::KEY_Tab            // Next item
cocos2d::KEY_Shift          // Alternative action
```

## Conditional Compilation (iOS Only)

### Location: src/main.cpp, top of file

```cpp
#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// All MFI controller code here
// This entire mod only compiles on iOS

#endif  // iOS check
```

## Common Debugging Patterns

### Logging Button Presses
```cpp
if (aPressed) {
    log::info("MFI: A button pressed in MenuLayer");
    this->keyDown(cocos2d::KEY_Return);
}
```

### Logging Connection
```cpp
bool connected = MFIControllerManager::isControllerConnected();
log::info("MFI: Controller connected = {}", connected);
```

### Logging State Values
```cpp
const auto& state = MFIControllerManager::getState();
log::info("MFI: Stick X={}, Y={}, A={}", 
    state.leftStickX, state.leftStickY, state.buttonA);
```

## Label Creation Error Checking

```cpp
auto* label = cocos2d::CCLabelBMFont::create(
    "A: PLAY  X: GALLERY  Y: CREATOR  B: EXIT  START: OPTIONS", 
    "goldFont.fnt"
);
if (label) {
    // Font loaded successfully
    label->setScale(0.4f);
    label->setAnchorPoint({0.5f, 0.f});
    auto vs = cocos2d::CCDirector::sharedDirector()->getWinSize();
    label->setPosition({vs.width * 0.5f, 10.f});
    label->setColor({255, 255, 255});
    this->addChild(label, 9999);
    m_fields->m_controllerHint = label;
} else {
    // Font not found - try fallback
    log::warn("MFI: goldFont.fnt not found, trying fallback");
    label = cocos2d::CCLabelBMFont::create(
        "A: PLAY  X: GALLERY  Y: CREATOR  B: EXIT  START: OPTIONS",
        "bigFont.fnt"
    );
}
```

## Complete MenuLayer Hook Template

```cpp
#include <Geode/modify/MenuLayer.hpp>

class $modify(MFIMenuLayer, MenuLayer) {
    struct Fields {
        bool m_prevA = false;
        bool m_prevB = false;
        bool m_prevX = false;
        bool m_prevY = false;
        bool m_prevMenu = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
        bool m_prevLeft = false;
        bool m_prevRight = false;
        cocos2d::CCLabelBMFont* m_controllerHint = nullptr;
    };

    bool init() {
        if (!MenuLayer::init()) return false;
        log::info("MFI: MenuLayer initialized");
        return true;
    }

    void update(float dt) {
        MenuLayer::update(dt);
        
        bool connected = MFIControllerManager::isControllerConnected();
        if (!connected) {
            updateHint();
            return;
        }
        
        const auto& state = MFIControllerManager::getState();
        
        // ... button handling here ...
        
        updateHint();
    }

    void updateHint() {
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected && !m_fields->m_controllerHint) {
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
            }
        } else if (!connected && m_fields->m_controllerHint) {
            m_fields->m_controllerHint->removeFromParentAndCleanup(true);
            m_fields->m_controllerHint = nullptr;
        }
    }
};
```

---

**Reference Updated**: Current session  
**Code Version**: Implementation Complete  
**Status**: Ready for macOS build
