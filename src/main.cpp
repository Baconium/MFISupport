#include <Geode/Geode.hpp>

using namespace geode::prelude;

#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Include our C++ interface (no Objective-C types)
#include "MFIController.h"

// Hook into PlayLayer for gameplay input
#include <Geode/modify/PlayLayer.hpp>

class $modify(MFIPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        log::info("=== MFI: PlayLayer::init() called ===");
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            log::error("MFI: PlayLayer::init() failed");
            return false;
        }
        log::info("=== MFI: PlayLayer::init() complete ===");
        return true;
    }

    void onEnter() {
        PlayLayer::onEnter();
        log::info("=== MFI: PlayLayer::onEnter() called ===");
    }

    void update(float dt) {
        PlayLayer::update(dt);
        
        // Just log to verify hook is running at all
        static int frameCount = 0;
        frameCount++;
        
        if (frameCount % 120 == 0) {
            log::info("=== MFI: PlayLayer::update() RUNNING (frame {}) ===", frameCount);
            
            bool connected = MFIControllerManager::isControllerConnected();
            log::info("MFI: Controller connected = {}", connected);
            
            if (connected) {
                const auto& state = MFIControllerManager::getState();
                log::info("MFI: State A={} B={} X={} Y={}", 
                    state.buttonA, state.buttonB, state.buttonX, state.buttonY);
            }
        }
    }

    void step(float dt) {
        PlayLayer::step(dt);
        
        static int stepCount = 0;
        stepCount++;
        
        if (stepCount % 120 == 0) {
            log::info("=== MFI: PlayLayer::step() RUNNING (frame {}) ===", stepCount);
        }
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
