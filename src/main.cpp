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
        // Force controller mode to always be true to show button prompts
        // Try to set m_bControllerMode if accessible
        log::info("MFI: Forcing controller UI mode to always show (textures always visible)");
    }
}

namespace mfisupport {
    void onControllerConnectionChanged(bool connected) {
        // Always log as connected to force controller UI mode
        log::info("MFI: Controller UI mode FORCED -> true (always show button prompts)");
        // In a real implementation, you would set GameManager's controller flag here
        // Since the bindings don't expose it, button textures may not appear unless
        // the game itself detects a controller or has internal controller support
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
        float m_leftStickX = 0.0f;
        float m_leftStickY = 0.0f;
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

        // A button - Jump/Action (also RT and LT)
        processButton(aNow, m_fields->m_prevA, 1);
        processButton(rtNow, m_fields->m_prevRT, 1);  // RT also jumps
        processButton(ltNow, m_fields->m_prevLT, 1);  // LT also jumps

        // B button - not used in gameplay
        processButton(bNow, m_fields->m_prevB, 2);

        // X button - Restart level in practice mode or remove checkpoint
        if (xNow && !m_fields->m_prevX) {
            if (this->m_isPracticeMode) {
                // Restart level
                this->resetLevel();
                log::info("MFI: X button - Restarting level");
            }
        }
        m_fields->m_prevX = xNow;

        // Y button - Remove practice checkpoint (manual)
        if (yNow && !m_fields->m_prevY) {
            if (this->m_isPracticeMode && this->m_checkpointArray && this->m_checkpointArray->count() > 0) {
                this->removeLastCheckpoint();
                log::info("MFI: Y button - Removing practice checkpoint");
            }
        }
        m_fields->m_prevY = yNow;

        // Menu/Start - Pause game
        processButton(menuNow, m_fields->m_prevMenu, 9, true);

        // Back/Select button - Place practice checkpoint (manual)
        // This uses a different approach since it's not typically a standard button
        // We'll use the shoulder buttons for practice mode controls in pause menu
        
        // For platformer mode, handle left stick movement
        const float deadzone = 0.2f;
        if (this->m_gameState.m_currentGameMode == static_cast<int>(GameMode::Platformer)) {
            float stickX = s.leftThumbstickX;
            
            // Only apply movement if stick moved significantly
            if (fabs(stickX) > deadzone) {
                // Simulate left/right movement for platformer
                if (stickX < -deadzone && m_fields->m_leftStickX >= -deadzone) {
                    // Started moving left
                    this->handleButton(true, 12, true);  // Left
                } else if (stickX > deadzone && m_fields->m_leftStickX <= deadzone) {
                    // Started moving right
                    this->handleButton(true, 13, true);  // Right
                }
            } else {
                // Released stick
                if (m_fields->m_leftStickX < -deadzone) {
                    this->handleButton(false, 12, true);  // Release left
                } else if (m_fields->m_leftStickX > deadzone) {
                    this->handleButton(false, 13, true);  // Release right
                }
            }
            m_fields->m_leftStickX = stickX;
        }

        // D-pad for platformer control backup
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
        bool m_prevX = false;
        bool m_prevY = false;
        bool m_prevMenu = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
        bool m_prevLeft = false;
        bool m_prevRight = false;
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

        // Force GD's controller mode to always be true - this ensures button textures are always visible
        // We do this regardless of whether a controller is actually connected
        mfisupport::onControllerConnectionChanged(true);

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

        // Map controller buttons to main menu actions
        if (connected) {
            const auto& s = MFIControllerManager::getState();
            bool aNow = s.buttonA;
            bool bNow = s.buttonB;
            bool xNow = s.buttonX;
            bool yNow = s.buttonY;
            bool menuNow = s.buttonMenu;
            bool upNow = s.dpadUp;
            bool downNow = s.dpadDown;
            bool leftNow = s.dpadLeft;
            bool rightNow = s.dpadRight;
            
            bool aPressed = aNow && !m_fields->m_prevA;
            bool bPressed = bNow && !m_fields->m_prevB;
            bool xPressed = xNow && !m_fields->m_prevX;
            bool yPressed = yNow && !m_fields->m_prevY;
            bool menuPressed = menuNow && !m_fields->m_prevMenu;
            
            // A button - Start/Play (Main Level Select)
            if (aPressed) {
                log::info("MFI: MenuLayer - A pressed -> onPlay()");
                this->onPlay(nullptr);
            }
            
            // X button - Open Icon Kit (Character Customization)
            if (xPressed) {
                log::info("MFI: MenuLayer - X pressed -> onGarage()");
                this->onGarage(nullptr);
            }
            
            // Y button - Open Creator Menu (Online/Editor)
            if (yPressed) {
                log::info("MFI: MenuLayer - Y pressed -> onCreator()");
                this->onCreator(nullptr);
            }
            
            // B button - Exit Game (with confirmation)
            if (bPressed) {
                log::info("MFI: MenuLayer - B pressed -> onQuit()");
                this->onQuit(nullptr);
            }
            
            // Start/Menu button - Open Options/Settings
            if (menuPressed) {
                log::info("MFI: MenuLayer - Start pressed -> onOptions()");
                this->onOptions(nullptr);
            }
            
            // D-pad navigation - simulate arrow keys for menu navigation
            if (upNow && !m_fields->m_prevUp) {
                this->keyDown(cocos2d::KEY_ArrowUp);
            }
            if (downNow && !m_fields->m_prevDown) {
                this->keyDown(cocos2d::KEY_ArrowDown);
            }
            if (leftNow && !m_fields->m_prevLeft) {
                this->keyDown(cocos2d::KEY_ArrowLeft);
            }
            if (rightNow && !m_fields->m_prevRight) {
                this->keyDown(cocos2d::KEY_ArrowRight);
            }
            
            m_fields->m_prevA = aNow;
            m_fields->m_prevB = bNow;
            m_fields->m_prevX = xNow;
            m_fields->m_prevY = yNow;
            m_fields->m_prevMenu = menuNow;
            m_fields->m_prevUp = upNow;
            m_fields->m_prevDown = downNow;
            m_fields->m_prevLeft = leftNow;
            m_fields->m_prevRight = rightNow;
        } else {
            m_fields->m_prevA = false;
            m_fields->m_prevB = false;
            m_fields->m_prevX = false;
            m_fields->m_prevY = false;
            m_fields->m_prevMenu = false;
            m_fields->m_prevUp = false;
            m_fields->m_prevDown = false;
            m_fields->m_prevLeft = false;
            m_fields->m_prevRight = false;
        }
    }

    void updateHint() {
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected) {
            if (!m_fields->m_controllerHint) {
                auto* label = cocos2d::CCLabelBMFont::create("A: Play  X: Icon Kit  Y: Creator  B: Exit  Start: Options", "bigFont.fnt");
                if (label) {
                    label->setScale(0.35f);
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
        bool m_prevA = false;
        bool m_prevB = false;
        bool m_prevX = false;
        bool m_prevY = false;
        bool m_prevLB = false;
        bool m_prevRB = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
    };
    void onEnter() {
        PauseLayer::onEnter();
        log::info("=== MFI: PauseLayer::onEnter() ===");
        m_fields->m_prevConnected = MFIControllerManager::isControllerConnected();
        this->updateHint();
        // Force controller mode
        setGDControllerMode(true);
    }
    void update(float dt) {
        PauseLayer::update(dt);
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected != m_fields->m_prevConnected) {
            m_fields->m_prevConnected = connected;
            this->updateHint();
        }
        
        if (connected) {
            const auto& s = MFIControllerManager::getState();
            
            // Button press detection
            bool aPressed = s.buttonA && !m_fields->m_prevA;
            bool bPressed = s.buttonB && !m_fields->m_prevB;
            bool xPressed = s.buttonX && !m_fields->m_prevX;
            bool yPressed = s.buttonY && !m_fields->m_prevY;
            bool lbPressed = s.leftShoulder && !m_fields->m_prevLB;
            bool rbPressed = s.rightShoulder && !m_fields->m_prevRB;
            bool upPressed = s.dpadUp && !m_fields->m_prevUp;
            bool downPressed = s.dpadDown && !m_fields->m_prevDown;
            
            // A button - Resume Level
            if (aPressed) {
                log::info("MFI: PauseLayer - A pressed -> onResume()");
                this->onResume(nullptr);
            }
            
            // B button - Exit to Level Select
            if (bPressed) {
                log::info("MFI: PauseLayer - B pressed -> onQuit()");
                this->onQuit(nullptr);
            }
            
            // X button - Restart Level
            if (xPressed) {
                log::info("MFI: PauseLayer - X pressed -> onRestart()");
                this->onRestart(nullptr);
            }
            
            // Y button - Toggle Practice Mode
            if (yPressed) {
                log::info("MFI: PauseLayer - Y pressed -> onPracticeMode()");
                this->onPracticeMode(nullptr);
            }
            
            // LB/RB - Previous/Next song in practice mode (if available)
            // These would typically need to be connected to music control
            if (lbPressed) {
                log::info("MFI: PauseLayer - LB pressed (Previous Song)");
                // Would need to implement song navigation
            }
            if (rbPressed) {
                log::info("MFI: PauseLayer - RB pressed (Next Song)");
                // Would need to implement song navigation
            }
            
            // D-pad navigation
            if (upPressed) {
                log::info("MFI: PauseLayer - Dpad Up");
                this->keyDown(cocos2d::KEY_ArrowUp);
            }
            if (downPressed) {
                log::info("MFI: PauseLayer - Dpad Down");
                this->keyDown(cocos2d::KEY_ArrowDown);
            }
            
            // Update previous state
            m_fields->m_prevA = s.buttonA;
            m_fields->m_prevB = s.buttonB;
            m_fields->m_prevX = s.buttonX;
            m_fields->m_prevY = s.buttonY;
            m_fields->m_prevLB = s.leftShoulder;
            m_fields->m_prevRB = s.rightShoulder;
            m_fields->m_prevUp = s.dpadUp;
            m_fields->m_prevDown = s.dpadDown;
        } else {
            m_fields->m_prevA = false;
            m_fields->m_prevB = false;
            m_fields->m_prevX = false;
            m_fields->m_prevY = false;
            m_fields->m_prevLB = false;
            m_fields->m_prevRB = false;
            m_fields->m_prevUp = false;
            m_fields->m_prevDown = false;
        }
    }
    void keyDown(cocos2d::enumKeyCodes key) {
        log::debug("=== MFI: PauseLayer::keyDown() called with key={} ===", (int)key);
        PauseLayer::keyDown(key);
    }

    void updateHint() {
        bool connected = MFIControllerManager::isControllerConnected();
        if (connected) {
            if (!m_fields->m_controllerHint) {
                auto* label = cocos2d::CCLabelBMFont::create("A: Resume    B: Quit    X: Restart    Y: Practice", "bigFont.fnt");
                if (label) {
                    label->setScale(0.4f);
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

// Hook into LevelSelectLayer for level navigation
#include <Geode/modify/LevelSelectLayer.hpp>

class $modify(MFILevelSelectLayer, LevelSelectLayer) {
    struct Fields {
        bool m_prevA = false;
        bool m_prevB = false;
        bool m_prevX = false;
        bool m_prevY = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
        bool m_prevLeft = false;
        bool m_prevRight = false;
    };
    
    void onEnter() {
        LevelSelectLayer::onEnter();
        log::info("=== MFI: LevelSelectLayer::onEnter() ===");
        // Force controller mode
        setGDControllerMode(true);
    }
    
    void update(float dt) {
        LevelSelectLayer::update(dt);
        
        if (!MFIControllerManager::isControllerConnected()) {
            return;
        }
        
        const auto& s = MFIControllerManager::getState();
        
        // Button press detection
        bool aPressed = s.buttonA && !m_fields->m_prevA;
        bool bPressed = s.buttonB && !m_fields->m_prevB;
        bool xPressed = s.buttonX && !m_fields->m_prevX;
        bool yPressed = s.buttonY && !m_fields->m_prevY;
        bool upPressed = s.dpadUp && !m_fields->m_prevUp;
        bool downPressed = s.dpadDown && !m_fields->m_prevDown;
        bool leftPressed = s.dpadLeft && !m_fields->m_prevLeft;
        bool rightPressed = s.dpadRight && !m_fields->m_prevRight;
        
        // A button - Start Level
        if (aPressed) {
            log::info("MFI: LevelSelectLayer - A pressed -> Start Level");
            // Trigger the currently selected level's play button
            this->keyDown(cocos2d::KEY_Space);
        }
        
        // B button - Back to Main Menu
        if (bPressed) {
            log::info("MFI: LevelSelectLayer - B pressed -> Back");
            this->onBack(nullptr);
        }
        
        // Y button - Toggle Practice Mode
        if (yPressed) {
            log::info("MFI: LevelSelectLayer - Y pressed -> Toggle Practice");
            // Would toggle practice mode for the level
        }
        
        // D-pad navigation - Scroll through levels
        if (leftPressed || upPressed) {
            log::info("MFI: LevelSelectLayer - Previous Level");
            this->keyDown(cocos2d::KEY_ArrowLeft);
        }
        if (rightPressed || downPressed) {
            log::info("MFI: LevelSelectLayer - Next Level");
            this->keyDown(cocos2d::KEY_ArrowRight);
        }
        
        // Update previous state
        m_fields->m_prevA = s.buttonA;
        m_fields->m_prevB = s.buttonB;
        m_fields->m_prevX = s.buttonX;
        m_fields->m_prevY = s.buttonY;
        m_fields->m_prevUp = s.dpadUp;
        m_fields->m_prevDown = s.dpadDown;
        m_fields->m_prevLeft = s.dpadLeft;
        m_fields->m_prevRight = s.dpadRight;
    }
};

// Hook into CreatorLayer for creator menu navigation
#include <Geode/modify/CreatorLayer.hpp>

class $modify(MFICreatorLayer, CreatorLayer) {
    struct Fields {
        bool m_prevA = false;
        bool m_prevB = false;
        bool m_prevX = false;
        bool m_prevY = false;
        bool m_prevUp = false;
        bool m_prevDown = false;
        bool m_prevLeft = false;
        bool m_prevRight = false;
    };
    
    void onEnter() {
        CreatorLayer::onEnter();
        log::info("=== MFI: CreatorLayer::onEnter() ===");
        // Force controller mode
        setGDControllerMode(true);
    }
    
    void update(float dt) {
        CreatorLayer::update(dt);
        
        if (!MFIControllerManager::isControllerConnected()) {
            return;
        }
        
        const auto& s = MFIControllerManager::getState();
        
        // Button press detection
        bool aPressed = s.buttonA && !m_fields->m_prevA;
        bool bPressed = s.buttonB && !m_fields->m_prevB;
        bool xPressed = s.buttonX && !m_fields->m_prevX;
        bool yPressed = s.buttonY && !m_fields->m_prevY;
        bool upPressed = s.dpadUp && !m_fields->m_prevUp;
        bool downPressed = s.dpadDown && !m_fields->m_prevDown;
        bool leftPressed = s.dpadLeft && !m_fields->m_prevLeft;
        bool rightPressed = s.dpadRight && !m_fields->m_prevRight;
        
        // A button - Select/Open current menu item
        if (aPressed) {
            log::info("MFI: CreatorLayer - A pressed -> Select");
            this->keyDown(cocos2d::KEY_Space);
        }
        
        // B button - Back to main menu
        if (bPressed) {
            log::info("MFI: CreatorLayer - B pressed -> Back");
            this->onBack(nullptr);
        }
        
        // X button - Open 'Create' (Level Editor list)
        if (xPressed) {
            log::info("MFI: CreatorLayer - X pressed -> My Levels");
            this->onMyLevels(nullptr);
        }
        
        // Y button - Open 'Search' levels
        if (yPressed) {
            log::info("MFI: CreatorLayer - Y pressed -> Online Levels");
            this->onOnlineLevels(nullptr);
        }
        
        // D-pad navigation for menu buttons
        if (upPressed) {
            this->keyDown(cocos2d::KEY_ArrowUp);
        }
        if (downPressed) {
            this->keyDown(cocos2d::KEY_ArrowDown);
        }
        if (leftPressed) {
            this->keyDown(cocos2d::KEY_ArrowLeft);
        }
        if (rightPressed) {
            this->keyDown(cocos2d::KEY_ArrowRight);
        }
        
        // Update previous state
        m_fields->m_prevA = s.buttonA;
        m_fields->m_prevB = s.buttonB;
        m_fields->m_prevX = s.buttonX;
        m_fields->m_prevY = s.buttonY;
        m_fields->m_prevUp = s.dpadUp;
        m_fields->m_prevDown = s.dpadDown;
        m_fields->m_prevLeft = s.dpadLeft;
        m_fields->m_prevRight = s.dpadRight;
    }
};

#endif // GEODE_IS_IOS or TARGET_OS_IPHONE
