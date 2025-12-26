#pragma once

#ifdef GEODE_IS_IOS

// Pure C++ interface - no Objective-C types
struct ControllerState {
    bool isConnected = false;
    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
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

// C++ interface to Objective-C++ implementation
class MFIControllerManager {
public:
    static void initialize();
    static const ControllerState& getState();
    static bool isControllerConnected();
};

#endif // GEODE_IS_IOS
