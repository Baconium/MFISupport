#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Import Objective-C frameworks FIRST, before any Cocos2d headers
#import <GameController/GameController.h>
#import <Foundation/Foundation.h>

// Then include our C++ interface
#include "MFIController.h"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

// Static storage for controller state
static ControllerState s_controllerState;
static GCController* s_currentController = nil;

static void connectController(GCController* controller) {
    log::info("=== MFI Controller: connectController() called ===");
    log::info("MFI Controller name: {}", [controller.vendorName UTF8String]);
    log::info("MFI Controller product category: {}", [controller.productCategory UTF8String]);
    
    if (controller.extendedGamepad) {
        log::info("MFI Controller supports extended gamepad profile");
    } else if (controller.gamepad) {
        log::info("MFI Controller supports standard gamepad profile");
    } else if (controller.microGamepad) {
        log::info("MFI Controller supports micro gamepad profile");
    }
    
    s_currentController = controller;
    s_controllerState.isConnected = true;
    log::info("MFI Controller connection status: isConnected = true");

    // Notify C++ side so the game can switch to controller UI mode (PC-like prompts)
    mfisupport::onControllerConnectionChanged(true);

    // iOS "pause/menu" button (matches Start on many controllers)
    controller.controllerPausedHandler = ^(GCController* c) {
        (void)c;
        s_controllerState.buttonMenu = true;
        log::info("MFI Controller Menu/Start triggered");
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.10 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            s_controllerState.buttonMenu = false;
        });
    };
    
    // Set up input handlers for extended gamepad profile
    if (controller.extendedGamepad) {
        GCExtendedGamepad* gamepad = controller.extendedGamepad;
        log::info("MFI: Setting up extended gamepad handlers...");
        
        // Button A (Jump/Select)
        log::info("MFI: Setting up Button A handler");
        gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.buttonA = pressed;
            if (pressed) {
                log::info("MFI Controller Button A triggered");
            }
        };
        
        // Button B (Back/Cancel)
        log::info("MFI: Setting up Button B handler");
        gamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.buttonB = pressed;
            if (pressed) {
                log::info("MFI Controller Button B triggered");
            }
        };
        
        // Button X
        log::info("MFI: Setting up Button X handler");
        gamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.buttonX = pressed;
            if (pressed) {
                log::info("MFI Controller Button X triggered");
            }
        };
        
        // Button Y
        log::info("MFI: Setting up Button Y handler");
        gamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.buttonY = pressed;
            if (pressed) {
                log::info("MFI Controller Button Y triggered");
            }
        };
        
        // Shoulder buttons
        log::info("MFI: Setting up Left Shoulder handler");
        gamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.leftShoulder = pressed;
            if (pressed) {
                log::info("MFI Controller Left Shoulder triggered");
            }
        };
        
        log::info("MFI: Setting up Right Shoulder handler");
        gamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.rightShoulder = pressed;
            if (pressed) {
                log::info("MFI Controller Right Shoulder triggered");
            }
        };
        
        // Triggers
        log::info("MFI: Setting up Left Trigger handler");
        gamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.leftTrigger = value > 0.5f;
            if (value > 0.5f) {
                log::info("MFI Controller Left Trigger triggered (value={})", value);
            }
        };
        
        log::info("MFI: Setting up Right Trigger handler");
        gamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.rightTrigger = value > 0.5f;
            if (value > 0.5f) {
                log::info("MFI Controller Right Trigger triggered (value={})", value);
            }
        };
        
        // D-pad
        log::info("MFI: Setting up D-pad handlers");
        gamepad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.dpadUp = pressed;
            if (pressed) {
                log::info("MFI Controller D-pad Up triggered");
            }
        };
        
        gamepad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.dpadDown = pressed;
            if (pressed) {
                log::info("MFI Controller D-pad Down triggered");
            }
        };
        
        gamepad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.dpadLeft = pressed;
            if (pressed) {
                log::info("MFI Controller D-pad Left triggered");
            }
        };
        
        gamepad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
            s_controllerState.dpadRight = pressed;
            if (pressed) {
                log::info("MFI Controller D-pad Right triggered");
            }
        };
        
        // Thumbsticks
        log::info("MFI: Setting up Left Thumbstick handler");
        gamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad* dpad, float xValue, float yValue) {
            s_controllerState.leftThumbstickX = xValue;
            s_controllerState.leftThumbstickY = yValue;
            if (xValue != 0.0f || yValue != 0.0f) {
                log::info("MFI Controller Left Thumbstick moved (x={}, y={})", xValue, yValue);
            }
        };
        
        log::info("MFI: Setting up Right Thumbstick handler");
        gamepad.rightThumbstick.valueChangedHandler = ^(GCControllerDirectionPad* dpad, float xValue, float yValue) {
            s_controllerState.rightThumbstickX = xValue;
            s_controllerState.rightThumbstickY = yValue;
            if (xValue != 0.0f || yValue != 0.0f) {
                log::info("MFI Controller Right Thumbstick moved (x={}, y={})", xValue, yValue);
            }
        };
        
        log::info("MFI: All extended gamepad handlers configured successfully");
    }
}

static void disconnectController() {
    log::info("=== MFI Controller: disconnectController() called ===");
    log::info("MFI Controller disconnected");
    if (s_currentController) {
        log::info("MFI Controller was: {}", [s_currentController.vendorName UTF8String]);
    }
    s_currentController = nil;
    s_controllerState = ControllerState();
    log::info("MFI Controller connection status: isConnected = false");

    // Notify C++ side so the game can switch back to touch UI mode
    mfisupport::onControllerConnectionChanged(false);
}

void MFIControllerManager::initialize() {
    log::info("=== MFI Controller Manager: Initialize Called ===");
    
    // Check for already connected controllers
    NSArray<GCController*>* controllers = [GCController controllers];
    log::info("MFI: Checking for already-connected controllers (count={})", controllers.count);
    
    if (controllers.count > 0) {
        log::info("MFI: Found {} connected controller(s)", controllers.count);
        connectController(controllers[0]);
    } else {
        log::info("MFI: No controllers currently connected");
    }
    
    // Register for controller connect notifications
    log::info("MFI: Registering for GCControllerDidConnectNotification");
    [[NSNotificationCenter defaultCenter] 
        addObserverForName:GCControllerDidConnectNotification
        object:nil
        queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification* notification) {
            log::info("MFI: GCControllerDidConnectNotification received");
            GCController* controller = (GCController*)notification.object;
            connectController(controller);
        }];
    
    // Register for controller disconnect notifications
    log::info("MFI: Registering for GCControllerDidDisconnectNotification");
    [[NSNotificationCenter defaultCenter]
        addObserverForName:GCControllerDidDisconnectNotification
        object:nil
        queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification* notification) {
            log::info("MFI: GCControllerDidDisconnectNotification received");
            disconnectController();
        }];
    
    log::info("=== MFI Controller Manager: Initialize Complete ===");
}

const ControllerState& MFIControllerManager::getState() {
    return s_controllerState;
}

bool MFIControllerManager::isControllerConnected() {
    return s_controllerState.isConnected;
}

#endif // GEODE_IS_IOS or TARGET_OS_IPHONE
