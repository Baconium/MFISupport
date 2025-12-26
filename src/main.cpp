#include <Geode/Geode.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_IOS

// Import iOS GameController framework
#import <GameController/GameController.h>
#import <Foundation/Foundation.h>

// Controller state structure
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

class MFIControllerManager {
private:
    static inline ControllerState s_controllerState;
    static inline GCController* s_currentController = nil;

public:
    static void initialize() {
        log::info("Initializing MFI Controller Support");
        
        // Check for already connected controllers
        NSArray<GCController*>* controllers = [GCController controllers];
        if (controllers.count > 0) {
            connectController(controllers[0]);
        }
        
        // Register for controller connect notifications
        [[NSNotificationCenter defaultCenter] 
            addObserverForName:GCControllerDidConnectNotification
            object:nil
            queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification* notification) {
                GCController* controller = (GCController*)notification.object;
                connectController(controller);
            }];
        
        // Register for controller disconnect notifications
        [[NSNotificationCenter defaultCenter]
            addObserverForName:GCControllerDidDisconnectNotification
            object:nil
            queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification* notification) {
                disconnectController();
            }];
    }
    
    static void connectController(GCController* controller) {
        log::info("MFI Controller connected: {}", [controller.vendorName UTF8String]);
        s_currentController = controller;
        s_controllerState.isConnected = true;
        
        // Set up input handlers for extended gamepad profile
        if (controller.extendedGamepad) {
            GCExtendedGamepad* gamepad = controller.extendedGamepad;
            
            // Button A (Jump/Select)
            gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.buttonA = pressed;
            };
            
            // Button B (Back/Cancel)
            gamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.buttonB = pressed;
            };
            
            // Button X
            gamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.buttonX = pressed;
            };
            
            // Button Y
            gamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.buttonY = pressed;
            };
            
            // Shoulder buttons
            gamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.leftShoulder = pressed;
            };
            
            gamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.rightShoulder = pressed;
            };
            
            // Triggers
            gamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.leftTrigger = value > 0.5f;
            };
            
            gamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.rightTrigger = value > 0.5f;
            };
            
            // D-pad
            gamepad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.dpadUp = pressed;
            };
            
            gamepad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.dpadDown = pressed;
            };
            
            gamepad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.dpadLeft = pressed;
            };
            
            gamepad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
                s_controllerState.dpadRight = pressed;
            };
            
            // Thumbsticks
            gamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad* dpad, float xValue, float yValue) {
                s_controllerState.leftThumbstickX = xValue;
                s_controllerState.leftThumbstickY = yValue;
            };
            
            gamepad.rightThumbstick.valueChangedHandler = ^(GCControllerDirectionPad* dpad, float xValue, float yValue) {
                s_controllerState.rightThumbstickX = xValue;
                s_controllerState.rightThumbstickY = yValue;
            };
        }
    }
    
    static void disconnectController() {
        log::info("MFI Controller disconnected");
        s_currentController = nil;
        s_controllerState = ControllerState();
    }
    
    static const ControllerState& getState() {
        return s_controllerState;
    }
    
    static bool isControllerConnected() {
        return s_controllerState.isConnected;
    }
};

// Hook into PlayLayer for gameplay input
#include <Geode/modify/PlayLayer.hpp>

class $modify(MFIPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        
        if (!MFIControllerManager::isControllerConnected()) {
            return;
        }
        
        const auto& state = MFIControllerManager::getState();
        
        // Handle jump input (Button A, Right Trigger, or D-pad Up)
        if (state.buttonA || state.rightTrigger || state.dpadUp) {
            this->pushButton(1, true);
        } else {
            this->releaseButton(1, true);
        }
        
        // Handle secondary actions (Button B or Left Trigger)
        if (state.buttonB || state.leftTrigger) {
            this->pushButton(1, false);
        } else {
            this->releaseButton(1, false);
        }
    }
};

// Hook into MenuLayer to show controller connection status
#include <Geode/modify/MenuLayer.hpp>

class $modify(MFIMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }
        
        // Initialize MFI controller support
        MFIControllerManager::initialize();
        
        return true;
    }
};

// Hook into PauseLayer for controller-friendly navigation
#include <Geode/modify/PauseLayer.hpp>

class $modify(MFIPauseLayer, PauseLayer) {
    static void onModify(auto& self) {
        (void) self.setHookPriority("PauseLayer::init", -1);
    }
    
    bool init() {
        if (!PauseLayer::init()) {
            return false;
        }
        
        if (MFIControllerManager::isControllerConnected()) {
            log::info("MFI Controller active in pause menu");
        }
        
        return true;
    }
    
    void keyDown(cocos2d::enumKeyCodes key) {
        PauseLayer::keyDown(key);
        
        if (!MFIControllerManager::isControllerConnected()) {
            return;
        }
        
        const auto& state = MFIControllerManager::getState();
        
        // Map controller buttons to menu navigation
        if (state.buttonA) {
            // A button acts as select/confirm
            this->keyDown(cocos2d::KEY_Space);
        }
        
        if (state.buttonB) {
            // B button acts as back/cancel
            this->onQuit(nullptr);
        }
    }
};

#endif // GEODE_IS_IOS
