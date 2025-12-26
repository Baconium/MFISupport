#include <Geode/Geode.hpp>

using namespace geode::prelude;

#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Include our C++ interface (no Objective-C types)
#include "MFIController.h"

// Hook into PlayLayer for gameplay input
#include <Geode/modify/PlayLayer.hpp>

class $modify(MFIPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        
        if (!MFIControllerManager::isControllerConnected()) {
            return;
        }
        
        // Log controller state periodically (every 30 frames)
        static int frameCounter = 0;
        frameCounter++;
        if (frameCounter >= 30) {
            frameCounter = 0;
            const auto& state = MFIControllerManager::getState();
            log::info("MFI PlayLayer state (periodic): A={} B={} X={} Y={} RT={} LT={} DpadU={} DpadD={} DpadL={} DpadR={} LSX={:.2f} LSY={:.2f} RSX={:.2f} RSY={:.2f}",
                state.buttonA, state.buttonB, state.buttonX, state.buttonY, 
                state.rightTrigger, state.leftTrigger, 
                state.dpadUp, state.dpadDown, state.dpadLeft, state.dpadRight,
                state.leftThumbstickX, state.leftThumbstickY,
                state.rightThumbstickX, state.rightThumbstickY);
        }
        
        const auto& state = MFIControllerManager::getState();
        
        // Log when specific buttons are detected
        if (state.buttonA) {
            log::debug("MFI PlayLayer: Button A detected");
        }
        if (state.buttonB) {
            log::debug("MFI PlayLayer: Button B detected");
        }
        if (state.buttonX) {
            log::debug("MFI PlayLayer: Button X detected");
        }
        if (state.buttonY) {
            log::debug("MFI PlayLayer: Button Y detected");
        }
        if (state.dpadUp) {
            log::debug("MFI PlayLayer: D-pad Up detected");
        }
        if (state.dpadDown) {
            log::debug("MFI PlayLayer: D-pad Down detected");
        }
        if (state.dpadLeft) {
            log::debug("MFI PlayLayer: D-pad Left detected");
        }
        if (state.dpadRight) {
            log::debug("MFI PlayLayer: D-pad Right detected");
        }
        if (state.leftTrigger) {
            log::debug("MFI PlayLayer: Left Trigger detected");
        }
        if (state.rightTrigger) {
            log::debug("MFI PlayLayer: Right Trigger detected");
        }
        
        // TODO: Map controller input to GD actions once button APIs are confirmed.
        // Currently we only log states to avoid compile issues on platforms where
        // button helpers are unavailable in bindings.
    }
};

// Hook into MenuLayer to show controller connection status
#include <Geode/modify/MenuLayer.hpp>

class $modify(MFIMenuLayer, MenuLayer) {
    bool init() {
        log::info("=== MFI: MenuLayer::init() called ===");
        
        if (!MenuLayer::init()) {
            log::error("MFI: MenuLayer::init() failed");
            return false;
        }
        
        log::info("=== MFI: MenuLayer::init() - calling MFIControllerManager::initialize() ===");
        
        // Initialize MFI controller support
        MFIControllerManager::initialize();
        
        log::info("=== MFI: MenuLayer::init() complete ===");
        
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
        log::info("=== MFI: PauseLayer::init() called ===");
        
        if (!PauseLayer::init(false)) {
            log::error("MFI: PauseLayer::init() failed");
            return false;
        }
        
        if (MFIControllerManager::isControllerConnected()) {
            log::info("=== MFI: PauseLayer::init() - Controller is active ===");
        } else {
            log::info("=== MFI: PauseLayer::init() - No controller connected ===");
        }
        
        return true;
    }
    
    void keyDown(cocos2d::enumKeyCodes key) {
        log::debug("=== MFI: PauseLayer::keyDown() called with key={} ===", (int)key);
        
        PauseLayer::keyDown(key);
        
        if (!MFIControllerManager::isControllerConnected()) {
            log::debug("MFI: PauseLayer::keyDown() - No controller connected");
            return;
        }
        
        const auto& state = MFIControllerManager::getState();
        
        // Map controller buttons to menu navigation
        if (state.buttonA) {
            log::info("MFI: PauseLayer::keyDown() - Button A detected (acting as select/confirm)");
            this->keyDown(cocos2d::KEY_Space);
        }
        
        if (state.buttonB) {
            log::info("MFI: PauseLayer::keyDown() - Button B detected (acting as back/cancel)");
            this->onQuit(nullptr);
        }
    }
};

#endif // GEODE_IS_IOS or TARGET_OS_IPHONE
