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

void MFIControllerManager::initialize() {
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

const ControllerState& MFIControllerManager::getState() {
    return s_controllerState;
}

bool MFIControllerManager::isControllerConnected() {
    return s_controllerState.isConnected;
}

#endif // GEODE_IS_IOS or TARGET_OS_IPHONE
