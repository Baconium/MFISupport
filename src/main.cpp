#include <Geode/Geode.hpp>

using namespace geode::prelude;

#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Include our C++ interface (no Objective-C types)
#include "MFIController.h"

// Hook into PlayLayer for gameplay input
#include <Geode/modify/PlayLayer.hpp>

class $modify(MFIPlayLayer, PlayLayer) {
    struct Fields {
        bool m_wasButtonAPressed = false;
        bool m_wasButtonBPressed = false;
    };

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
        
        // Log every update call periodically to verify hook is running
        static int totalFrames = 0;
        totalFrames++;
        if (totalFrames % 120 == 0) {
            log::info("=== MFI: PlayLayer::update() running (frame {}) ===", totalFrames);
        }
        
        bool controllerConnected = MFIControllerManager::isControllerConnected();
        if (!controllerConnected) {
            if (totalFrames % 120 == 0) {
                log::warn("MFI: No controller connected in update()");
            }
            return;
        }
        
        // Log that we have a controller
        if (totalFrames % 120 == 0) {
            log::info("MFI: Controller IS connected in update()");
        }
        
        const auto& state = MFIControllerManager::getState();
        
        // Button A -> Jump (simulate holding)
        if (state.buttonA && !m_fields->m_wasButtonAPressed) {
            log::info("=== MFI: Button A PRESSED - Triggering jump ===");
            this->handleButton(true, 1, true);
            m_fields->m_wasButtonAPressed = true;
        } else if (!state.buttonA && m_fields->m_wasButtonAPressed) {
            log::info("=== MFI: Button A RELEASED ===");
            this->handleButton(false, 1, true);
            m_fields->m_wasButtonAPressed = false;
        }
        
        // Button B -> Pause
        if (state.buttonB && !m_fields->m_wasButtonBPressed) {
            log::info("=== MFI: Button B PRESSED - Pausing game ===");
            this->pauseGame(false);
            m_fields->m_wasButtonBPressed = true;
        } else if (!state.buttonB) {
            m_fields->m_wasButtonBPressed = false;
        }
        
        // Log state periodically
        if (totalFrames % 60 == 0) {
            log::info("MFI PlayLayer state: A={} B={} X={} Y={}", 
                state.buttonA, state.buttonB, state.buttonX, state.buttonY);
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
