#include <Geode/Geode.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_IOS

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
        
        const auto& state = MFIControllerManager::getState();
        // TODO: Map controller input to GD actions once button APIs are confirmed.
        // Currently we only log states to avoid compile issues on platforms where
        // button helpers are unavailable in bindings.
        log::debug("MFI state: A={} B={} RT={} LT={} DpadU={}",
            state.buttonA, state.buttonB, state.rightTrigger, state.leftTrigger, state.dpadUp);
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
        if (!PauseLayer::init(false)) {
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
