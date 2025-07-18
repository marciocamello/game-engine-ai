#pragma once

#include "Core/Math.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <vector>

namespace GameEngine {
    class PhysicsEngine;
    
    namespace Physics {
        
        /**
         * @brief Console interface for physics debugging and parameter tuning
         */
        class PhysicsConsole {
        public:
            explicit PhysicsConsole(std::shared_ptr<PhysicsEngine> engine);
            ~PhysicsConsole() = default;
            
            /**
             * @brief Execute a console command
             * @param command The command string to execute
             * @return Result message from command execution
             */
            std::string ExecuteCommand(const std::string& command);
            
            /**
             * @brief Get list of available commands
             * @return Vector of command names with descriptions
             */
            std::vector<std::pair<std::string, std::string>> GetAvailableCommands() const;
            
            /**
             * @brief Get help text for all commands
             * @return Formatted help text
             */
            std::string GetHelpText() const;
            
        private:
            using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;
            
            std::shared_ptr<PhysicsEngine> m_engine;
            std::unordered_map<std::string, CommandHandler> m_commands;
            std::unordered_map<std::string, std::string> m_commandDescriptions;
            
            // Command implementations
            void RegisterCommands();
            std::vector<std::string> ParseCommand(const std::string& command);
            
            // Debug drawing commands
            std::string HandleEnableDebugDraw(const std::vector<std::string>& args);
            std::string HandleDisableDebugDraw(const std::vector<std::string>& args);
            std::string HandleSetDebugMode(const std::vector<std::string>& args);
            std::string HandleGetDebugMode(const std::vector<std::string>& args);
            
            // Physics parameter commands
            std::string HandleSetGravity(const std::vector<std::string>& args);
            std::string HandleGetGravity(const std::vector<std::string>& args);
            std::string HandleSetTimestep(const std::vector<std::string>& args);
            std::string HandleGetTimestep(const std::vector<std::string>& args);
            std::string HandleSetSolverIterations(const std::vector<std::string>& args);
            std::string HandleGetSolverIterations(const std::vector<std::string>& args);
            std::string HandleSetContactThresholds(const std::vector<std::string>& args);
            std::string HandleGetContactThresholds(const std::vector<std::string>& args);
            
            // Information commands
            std::string HandleDebugInfo(const std::vector<std::string>& args);
            std::string HandleHelp(const std::vector<std::string>& args);
            std::string HandleReset(const std::vector<std::string>& args);
            
            // Utility functions
            std::string FormatFloat(float value, int precision = 3) const;
            std::string FormatVec3(const Math::Vec3& vec, int precision = 3) const;
        };
        
    } // namespace Physics
} // namespace GameEngine