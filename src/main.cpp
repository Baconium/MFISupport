#include <Geode/Geode.hpp>

using namespace geode::prelude;

#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Include our C++ interface (no Objective-C types)
#include "MFIController.h"

// Hook into PlayLayer for gameplay input
#include <Geode/modify/PlayLayer.hpp>

class $modify(MFIPlayLayer, PlayLayer) {
    void onEnter() {
        PlayLayer::onEnter();
        log::info("=== MFI: PlayLayer::onEnter() ===");
    }
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
        
        // Log when specific buttons are detected (INFO for visibility)
        if (state.buttonA) log::info("MFI PlayLayer: Button A detected");
        if (state.buttonB) log::info("MFI PlayLayer: Button B detected");
        if (state.buttonX) log::info("MFI PlayLayer: Button X detected");
        if (state.buttonY) log::info("MFI PlayLayer: Button Y detected");
        if (state.dpadUp) log::info("MFI PlayLayer: D-pad Up detected");
        if (state.dpadDown) log::info("MFI PlayLayer: D-pad Down detected");
        if (state.dpadLeft) log::info("MFI PlayLayer: D-pad Left detected");
        if (state.dpadRight) log::info("MFI PlayLayer: D-pad Right detected");
        if (state.leftTrigger) log::info("MFI PlayLayer: Left Trigger detected");
        if (state.rightTrigger) log::info("MFI PlayLayer: Right Trigger detected");
    }
};

// Hook into MenuLayer to show controller connection status and periodic state
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
    void update(float dt) {
        MenuLayer::update(dt);
        static int menuFrameCounter = 0;
        if (MFIControllerManager::isControllerConnected()) {
            menuFrameCounter++;
            if (menuFrameCounter >= 60) {
                menuFrameCounter = 0;
                const auto& s = MFIControllerManager::getState();
                log::info("MFI MenuLayer state (periodic): A={} B={} DpadU={} DpadD={} DpadL={} DpadR={}",
                    s.buttonA, s.buttonB, s.dpadUp, s.dpadDown, s.dpadLeft, s.dpadRight);
            }
        }
    }
};

// Hook into PauseLayer for controller-friendly navigation
#include <Geode/modify/PauseLayer.hpp>

class $modify(MFIPauseLayer, PauseLayer) {
    void onEnter() {
        PauseLayer::onEnter();
        log::info("=== MFI: PauseLayer::onEnter() ===");
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
