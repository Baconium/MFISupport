#pragma once

#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Pure C++ interface - no Objective-C types
struct ControllerState {
    bool isConnected = false;
    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    // MFi "menu / start" style button (momentary)
    bool buttonMenu = false;
    bool leftShoulder = false;
    bool rightShoulder = false;
    bool leftTrigger = false;
    bool rightTrigger = false;
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;
    float leftThumbstickX = 0.0f;
    float leftThumbstickY = 0.0f;
    float rightThumbstickX = 0.0f;
    float rightThumbstickY = 0.0f;
};

namespace mfisupport {
    // Implemented on the C++ side (main.cpp) to toggle GD's internal controller mode.
    void onControllerConnectionChanged(bool connected);
}

// C++ interface to Objective-C++ implementation
class MFIControllerManager {
public:
    static void initialize();
    static const ControllerState& getState();
    static bool isControllerConnected();
};

#endif // GEODE_IS_IOS or TARGET_OS_IPHONE
