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

        // Compute press/release using previous state snapshot,
        // then mutate prev once to avoid missing releases.
        auto processButton = [&](bool now, bool& prev, int id, bool pauseOnPress = false) {
            bool was = prev;
            bool pressed = now && !was;
            bool released = !now && was;
            if (pressed) {
                this->handleButton(true, id, true);
                if (pauseOnPress) {
                    this->pauseGame(false);
                }
            }
            if (released) {
                this->handleButton(false, id, true);
            }
            prev = now;
        };

        // Feed ALL buttons into GD's controller pipeline (PC-like).
        // Button ids are based on common GD bindings conventions:
        // 1=A, 2=B, 3=X, 4=Y, 5=LB, 6=RB, 7=LT, 8=RT, 9=Menu/Start, 10=Up, 11=Down, 12=Left, 13=Right
        // If a specific id differs on your build, we can adjust after a quick test.

        // A (jump)
        processButton(aNow, m_fields->m_prevA, 1);

        // B
        processButton(bNow, m_fields->m_prevB, 2);

        // X
        processButton(xNow, m_fields->m_prevX, 3);

        // Y
        processButton(yNow, m_fields->m_prevY, 4);

        // Shoulders / triggers
        processButton(lbNow, m_fields->m_prevLB, 5);
        processButton(rbNow, m_fields->m_prevRB, 6);
        processButton(ltNow, m_fields->m_prevLT, 7);
        processButton(rtNow, m_fields->m_prevRT, 8);

        // Menu/Start should pause like PC
        processButton(menuNow, m_fields->m_prevMenu, 9, true);

        // D-pad
        processButton(upNow, m_fields->m_prevUp, 10);
        processButton(downNow, m_fields->m_prevDown, 11);
        processButton(leftNow, m_fields->m_prevLeft, 12);
        processButton(rightNow, m_fields->m_prevRight, 13);
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
    struct Fields {
        cocos2d::CCLabelBMFont* m_controllerHint = nullptr;
        bool m_prevConnected = false;
        bool m_prevA = false;
        bool m_prevB = false;
        cocos2d::CCNode* m_glyphOverlay = nullptr;
    };

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

        m_fields->m_prevConnected = MFIControllerManager::isControllerConnected();
        this->updateHint();
        this->updateGlyphOverlay();
        
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

        // Toggle hints when connection changes
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected != m_fields->m_prevConnected) {
            m_fields->m_prevConnected = connected;
            this->updateHint();
            this->updateGlyphOverlay();
        }

        // Map A/B press edges to common main-menu actions
        if (connected) {
            const auto& s = MFIControllerManager::getState();
            bool aNow = s.buttonA;
            bool bNow = s.buttonB;
            bool aPressed = aNow && !m_fields->m_prevA;
            bool bPressed = bNow && !m_fields->m_prevB;
            if (aPressed) {
                log::info("MFI: MenuLayer - A pressed -> onPlay()");
                this->onPlay(nullptr);
            }
            if (bPressed) {
                log::info("MFI: MenuLayer - B pressed (no action on main menu)");
            }
            m_fields->m_prevA = aNow;
            m_fields->m_prevB = bNow;
        } else {
            m_fields->m_prevA = false;
            m_fields->m_prevB = false;
        }
    }

    void updateHint() {
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected) {
            if (!m_fields->m_controllerHint) {
                auto* label = cocos2d::CCLabelBMFont::create("A: Select    B: Back", "bigFont.fnt");
                if (label) {
                    label->setScale(0.45f);
                    auto vs = cocos2d::CCDirector::sharedDirector()->getWinSize();
                    label->setAnchorPoint({1.f, 0.f});
                    label->setPosition({vs.width - 10.f, 10.f});
                    label->setColor({255, 255, 255});
                    this->addChild(label, 9999);
                    m_fields->m_controllerHint = label;
                    log::info("MFI: MenuLayer controller hints shown");
                }
            }
        } else {
            if (m_fields->m_controllerHint) {
                m_fields->m_controllerHint->removeFromParentAndCleanup(true);
                m_fields->m_controllerHint = nullptr;
                log::info("MFI: MenuLayer controller hints hidden");
            }
        }
    }

    cocos2d::CCSprite* createGlyph(char letter) {
        auto* circle = cocos2d::CCSprite::create("circle.png");
        if (!circle) return nullptr;
        circle->setScale(0.3f);
        auto* label = cocos2d::CCLabelBMFont::create(std::string(1, letter).c_str(), "bigFont.fnt");
        if (!label) return nullptr;
        label->setScale(0.4f);
        label->setColor({0,0,0});
        auto* node = cocos2d::CCNode::create();
        node->addChild(circle);
        label->setPosition(circle->getContentSize() * 0.5f);
        node->addChild(label);
        // Render node into a CCSprite via CCRenderTexture would be heavy; return circle with label attached
        // Caller should treat returned CCNode*; overload with CCSprite* for simplicity here.
        return reinterpret_cast<cocos2d::CCSprite*>(node);
    }

    void updateGlyphOverlay() {
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected) {
            if (!m_fields->m_glyphOverlay) {
                auto* overlay = cocos2d::CCNode::create();
                auto vs = cocos2d::CCDirector::sharedDirector()->getWinSize();
                // Create A/B glyphs
                auto* aGlyph = createGlyph('A');
                auto* bGlyph = createGlyph('B');
                if (aGlyph && bGlyph) {
                    aGlyph->setPosition({vs.width - 60.f, 22.f});
                    bGlyph->setPosition({vs.width - 30.f, 22.f});
                    overlay->addChild(aGlyph);
                    overlay->addChild(bGlyph);
                    // Add hint text
                    auto* hint = cocos2d::CCLabelBMFont::create("Select  Back", "bigFont.fnt");
                    hint->setScale(0.35f);
                    hint->setAnchorPoint({1.f,0.f});
                    hint->setPosition({vs.width - 70.f, 10.f});
                    overlay->addChild(hint);
                    this->addChild(overlay, 9998);
                    m_fields->m_glyphOverlay = overlay;
                    log::info("MFI: MenuLayer glyph overlay shown");
                }
            }
        } else {
            if (m_fields->m_glyphOverlay) {
                m_fields->m_glyphOverlay->removeFromParentAndCleanup(true);
                m_fields->m_glyphOverlay = nullptr;
                log::info("MFI: MenuLayer glyph overlay hidden");
            }
        }
    }
};

// Hook into PauseLayer for controller-friendly navigation
#include <Geode/modify/PauseLayer.hpp>

class $modify(MFIPauseLayer, PauseLayer) {
    struct Fields {
        cocos2d::CCLabelBMFont* m_controllerHint = nullptr;
        bool m_prevConnected = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
    };
    void onEnter() {
        PauseLayer::onEnter();
        log::info("=== MFI: PauseLayer::onEnter() ===");
        m_fields->m_prevConnected = MFIControllerManager::isControllerConnected();
        this->updateHint();
    }
    void update(float dt) {
        PauseLayer::update(dt);
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected != m_fields->m_prevConnected) {
            m_fields->m_prevConnected = connected;
            this->updateHint();
        }
        // D-pad navigation parity
        if (connected) {
            const auto& s = MFIControllerManager::getState();
            bool upPressed = s.dpadUp && !m_fields->m_prevUp;
            bool downPressed = s.dpadDown && !m_fields->m_prevDown;
            if (upPressed) {
                log::info("MFI: PauseLayer - Dpad Up -> KEY_UpArrow");
                this->keyDown(cocos2d::KEY_UpArrow);
            }
            if (downPressed) {
                log::info("MFI: PauseLayer - Dpad Down -> KEY_DownArrow");
                this->keyDown(cocos2d::KEY_DownArrow);
            }
            m_fields->m_prevUp = s.dpadUp;
            m_fields->m_prevDown = s.dpadDown;
        } else {
            m_fields->m_prevUp = false;
            m_fields->m_prevDown = false;
        }
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

    void updateHint() {
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected) {
            if (!m_fields->m_controllerHint) {
                auto* label = cocos2d::CCLabelBMFont::create("A: Resume    B: Quit", "bigFont.fnt");
                if (label) {
                    label->setScale(0.5f);
                    auto vs = cocos2d::CCDirector::sharedDirector()->getWinSize();
                    label->setAnchorPoint({0.5f, 0.f});
                    label->setPosition({vs.width * 0.5f, 10.f});
                    label->setColor({255, 255, 255});
                    this->addChild(label, 9999);
                    m_fields->m_controllerHint = label;
                    log::info("MFI: PauseLayer controller hints shown");
                }
            }
        } else {
            if (m_fields->m_controllerHint) {
                m_fields->m_controllerHint->removeFromParentAndCleanup(true);
                m_fields->m_controllerHint = nullptr;
                log::info("MFI: PauseLayer controller hints hidden");
            }
        }
    }
};

#endif // GEODE_IS_IOS or TARGET_OS_IPHONE
