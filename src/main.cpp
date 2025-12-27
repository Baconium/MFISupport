#include <Geode/Geode.hpp>

using namespace geode::prelude;

#if defined(GEODE_IS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)

// Include our C++ interface (no Objective-C types)
#include "MFIController.h"

// Best-effort toggling of GD's internal controller mode (drives controller prompts / textures)
#include <Geode/binding/GameManager.hpp>

namespace {
    static void setGDControllerMode(bool connected) {
        auto* gm = GameManager::sharedState();
        if (!gm) {
            log::warn("MFI: GameManager::sharedState() is null");
            return;
        }
        // The 2.2074 bindings in this build don’t expose the usual controller flags.
        // We log here for visibility; prompts may not swap unless a future binding adds a setter/field.
        log::info("MFI: Requesting controller UI mode -> {} (no exposed GameManager flag in this binding)", connected);
    }
}

namespace mfisupport {
    void onControllerConnectionChanged(bool connected) {
        log::info("MFI: Controller connection changed -> {}", connected);
        setGDControllerMode(connected);
    }
}

// Hook into PlayLayer for gameplay input
#include <Geode/modify/PlayLayer.hpp>

class $modify(MFIPlayLayer, PlayLayer) {
    struct Fields {
        bool m_tickScheduled = false;
        bool m_prevA = false;
        bool m_prevB = false;
        bool m_prevX = false;
        bool m_prevY = false;
        bool m_prevLB = false;
        bool m_prevRB = false;
        bool m_prevLT = false;
        bool m_prevRT = false;
        bool m_prevMenu = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
        bool m_prevLeft = false;
        bool m_prevRight = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        log::info("=== MFI: PlayLayer::init() called ===");
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            log::error("MFI: PlayLayer::init() failed");
            return false;
        }

        // Schedule from init() since PlayLayer::onEnter() may be unhookable on this version
        if (!m_fields->m_tickScheduled) {
            this->schedule(schedule_selector(MFIPlayLayer::mfiTick));
            m_fields->m_tickScheduled = true;
            log::info("=== MFI: PlayLayer::mfiTick scheduled (from init) ===");
        }

        log::info("=== MFI: PlayLayer::init() complete ===");
        return true;
    }

    void onEnter() {
        PlayLayer::onEnter();
        log::info("=== MFI: PlayLayer::onEnter() called ===");
        // Best-effort fallback in case init() scheduling didn't happen
        if (!m_fields->m_tickScheduled) {
            this->schedule(schedule_selector(MFIPlayLayer::mfiTick));
            m_fields->m_tickScheduled = true;
            log::info("=== MFI: PlayLayer::mfiTick scheduled (from onEnter) ===");
        }
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
    
    void mfiTick(float dt) {
        // Our own scheduled per-frame callback; safe alternative to hooking update()
        static int tickCount = 0;
        tickCount++;
        if (tickCount == 1) {
            log::info("=== MFI: PlayLayer::mfiTick() FIRST CALL ===");
        }
        if (tickCount % 120 == 0) {
            log::info("=== MFI: PlayLayer::mfiTick() RUNNING (frame {}) ===", tickCount);
            bool connected = MFIControllerManager::isControllerConnected();
            log::info("MFI: Controller connected = {}", connected);
        }
        if (!MFIControllerManager::isControllerConnected()) {
            return;
        }

        const auto& s = MFIControllerManager::getState();

        const bool aNow = s.buttonA;
        const bool bNow = s.buttonB;
        const bool xNow = s.buttonX;
        const bool yNow = s.buttonY;
        const bool lbNow = s.leftShoulder;
        const bool rbNow = s.rightShoulder;
        const bool ltNow = s.leftTrigger;
        const bool rtNow = s.rightTrigger;
        const bool menuNow = s.buttonMenu;
        const bool upNow = s.dpadUp;
        const bool downNow = s.dpadDown;
        const bool leftNow = s.dpadLeft;
        const bool rightNow = s.dpadRight;

        auto edge = [&](bool now, bool& prev) {
            bool pressed = now && !prev;
            prev = now;
            return pressed;
        };
        auto edgeRelease = [&](bool now, bool& prev) {
            bool released = !now && prev;
            prev = now;
            return released;
        };

        // Feed ALL buttons into GD's controller pipeline (PC-like).
        // Button ids are based on common GD bindings conventions:
        // 1=A, 2=B, 3=X, 4=Y, 5=LB, 6=RB, 7=LT, 8=RT, 9=Menu/Start, 10=Up, 11=Down, 12=Left, 13=Right
        // If a specific id differs on your build, we can adjust after a quick test.

        // A (jump)
        if (edge(aNow, m_fields->m_prevA)) {
            this->handleButton(true, 1, true);
        }
        if (edgeRelease(aNow, m_fields->m_prevA)) {
            this->handleButton(false, 1, true);
        }

        // B
        if (edge(bNow, m_fields->m_prevB)) {
            this->handleButton(true, 2, true);
        }
        if (edgeRelease(bNow, m_fields->m_prevB)) {
            this->handleButton(false, 2, true);
        }

        // X
        if (edge(xNow, m_fields->m_prevX)) {
            this->handleButton(true, 3, true);
        }
        if (edgeRelease(xNow, m_fields->m_prevX)) {
            this->handleButton(false, 3, true);
        }

        // Y
        if (edge(yNow, m_fields->m_prevY)) {
            this->handleButton(true, 4, true);
        }
        if (edgeRelease(yNow, m_fields->m_prevY)) {
            this->handleButton(false, 4, true);
        }

        // Shoulders / triggers
        if (edge(lbNow, m_fields->m_prevLB)) this->handleButton(true, 5, true);
        if (edgeRelease(lbNow, m_fields->m_prevLB)) this->handleButton(false, 5, true);
        if (edge(rbNow, m_fields->m_prevRB)) this->handleButton(true, 6, true);
        if (edgeRelease(rbNow, m_fields->m_prevRB)) this->handleButton(false, 6, true);
        if (edge(ltNow, m_fields->m_prevLT)) this->handleButton(true, 7, true);
        if (edgeRelease(ltNow, m_fields->m_prevLT)) this->handleButton(false, 7, true);
        if (edge(rtNow, m_fields->m_prevRT)) this->handleButton(true, 8, true);
        if (edgeRelease(rtNow, m_fields->m_prevRT)) this->handleButton(false, 8, true);

        // Menu/Start should pause like PC
        if (edge(menuNow, m_fields->m_prevMenu)) {
            this->handleButton(true, 9, true);
            this->pauseGame(false);
        }
        if (edgeRelease(menuNow, m_fields->m_prevMenu)) {
            this->handleButton(false, 9, true);
        }

        // D-pad
        if (edge(upNow, m_fields->m_prevUp)) this->handleButton(true, 10, true);
        if (edgeRelease(upNow, m_fields->m_prevUp)) this->handleButton(false, 10, true);
        if (edge(downNow, m_fields->m_prevDown)) this->handleButton(true, 11, true);
        if (edgeRelease(downNow, m_fields->m_prevDown)) this->handleButton(false, 11, true);
        if (edge(leftNow, m_fields->m_prevLeft)) this->handleButton(true, 12, true);
        if (edgeRelease(leftNow, m_fields->m_prevLeft)) this->handleButton(false, 12, true);
        if (edge(rightNow, m_fields->m_prevRight)) this->handleButton(true, 13, true);
        if (edgeRelease(rightNow, m_fields->m_prevRight)) this->handleButton(false, 13, true);
    }

    void onExit() {
        // Cleanup: unschedule our tick when leaving the layer
        if (m_fields->m_tickScheduled) {
            this->unschedule(schedule_selector(MFIPlayLayer::mfiTick));
            m_fields->m_tickScheduled = false;
        }
        PlayLayer::onExit();
        log::info("=== MFI: PlayLayer::onExit() - mfiTick unscheduled ===");
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

        // Force GD's controller mode based on current connection state (drives PC-like prompts)
        mfisupport::onControllerConnectionChanged(MFIControllerManager::isControllerConnected());
        
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
