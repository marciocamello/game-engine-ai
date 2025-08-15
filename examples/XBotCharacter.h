#pragma once

#include "Game/Character.h"

namespace GameExample {

    /**
     * @brief XBot character implementation with specific animations and state machine
     * 
     * This class extends the base Character class to provide XBot-specific animation
     * logic, asset loading, and state machine configuration. It demonstrates how
     * to create project-specific character implementations while keeping the base
     * engine modular and asset-agnostic.
     */
    class XBotCharacter : public GameEngine::Character {
    public:
        XBotCharacter();
        virtual ~XBotCharacter();

        // Character type identification
        const char* GetCharacterType() const { return "XBot"; }

    protected:
        // Override virtual methods from base Character class
        virtual bool LoadCharacterAnimations() override;
        virtual void SetupCharacterAnimationStateMachine() override;

    private:
        // XBot-specific animation loading
        bool LoadXBotAnimationFromFBX(const std::string& fbxPath, const std::string& animationName);
        void CreateXBotStateMachine();
        void SetupXBotAnimationParameters();

        // XBot animation state tracking
        std::string m_currentXBotState = "Idle";
        bool m_xbotAnimationsLoaded = false;
    };

} // namespace GameExample