#include "Physics/PhysicsConsole.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Core/Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace GameEngine {
    namespace Physics {
        
        PhysicsConsole::PhysicsConsole(std::shared_ptr<PhysicsEngine> engine)
            : m_engine(engine) {
            RegisterCommands();
        }
        
        std::string PhysicsConsole::ExecuteCommand(const std::string& command) {
            if (command.empty()) {
                return "Empty command. Type 'help' for available commands.";
            }
            
            auto args = ParseCommand(command);
            if (args.empty()) {
                return "Invalid command format.";
            }
            
            std::string commandName = args[0];
            std::transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);
            
            auto it = m_commands.find(commandName);
            if (it != m_commands.end()) {
                try {
                    return it->second(args);
                } catch (const std::exception& e) {
                    return "Command execution error: " + std::string(e.what());
                }
            }
            
            return "Unknown command: " + commandName + ". Type 'help' for available commands.";
        }
        
        std::vector<std::pair<std::string, std::string>> PhysicsConsole::GetAvailableCommands() const {
            std::vector<std::pair<std::string, std::string>> commands;
            for (const auto& [name, description] : m_commandDescriptions) {
                commands.emplace_back(name, description);
            }
            return commands;
        }
        
        std::string PhysicsConsole::GetHelpText() const {
            std::ostringstream oss;
            oss << "Available Physics Console Commands:\n";
            oss << "=====================================\n\n";
            
            for (const auto& [name, description] : m_commandDescriptions) {
                oss << std::left << std::setw(25) << name << " - " << description << "\n";
            }
            
            return oss.str();
        }
        
        void PhysicsConsole::RegisterCommands() {
            // Debug drawing commands
            m_commands["enable_debug_draw"] = [this](const auto& args) { return HandleEnableDebugDraw(args); };
            m_commandDescriptions["enable_debug_draw"] = "Enable physics debug drawing";
            
            m_commands["disable_debug_draw"] = [this](const auto& args) { return HandleDisableDebugDraw(args); };
            m_commandDescriptions["disable_debug_draw"] = "Disable physics debug drawing";
            
            m_commands["set_debug_mode"] = [this](const auto& args) { return HandleSetDebugMode(args); };
            m_commandDescriptions["set_debug_mode"] = "Set debug mode (wireframe, aabb, contacts, all)";
            
            m_commands["get_debug_mode"] = [this](const auto& args) { return HandleGetDebugMode(args); };
            m_commandDescriptions["get_debug_mode"] = "Get current debug mode";
            
            // Physics parameter commands
            m_commands["set_gravity"] = [this](const auto& args) { return HandleSetGravity(args); };
            m_commandDescriptions["set_gravity"] = "Set gravity vector (x y z)";
            
            m_commands["get_gravity"] = [this](const auto& args) { return HandleGetGravity(args); };
            m_commandDescriptions["get_gravity"] = "Get current gravity vector";
            
            m_commands["set_timestep"] = [this](const auto& args) { return HandleSetTimestep(args); };
            m_commandDescriptions["set_timestep"] = "Set physics timestep (seconds)";
            
            m_commands["get_timestep"] = [this](const auto& args) { return HandleGetTimestep(args); };
            m_commandDescriptions["get_timestep"] = "Get current physics timestep";
            
            m_commands["set_solver_iterations"] = [this](const auto& args) { return HandleSetSolverIterations(args); };
            m_commandDescriptions["set_solver_iterations"] = "Set solver iterations count";
            
            m_commands["get_solver_iterations"] = [this](const auto& args) { return HandleGetSolverIterations(args); };
            m_commandDescriptions["get_solver_iterations"] = "Get current solver iterations";
            
            m_commands["set_contact_thresholds"] = [this](const auto& args) { return HandleSetContactThresholds(args); };
            m_commandDescriptions["set_contact_thresholds"] = "Set contact thresholds (breaking processing)";
            
            m_commands["get_contact_thresholds"] = [this](const auto& args) { return HandleGetContactThresholds(args); };
            m_commandDescriptions["get_contact_thresholds"] = "Get current contact thresholds";
            
            // Information commands
            m_commands["debug_info"] = [this](const auto& args) { return HandleDebugInfo(args); };
            m_commandDescriptions["debug_info"] = "Display physics debug information";
            
            m_commands["help"] = [this](const auto& args) { return HandleHelp(args); };
            m_commandDescriptions["help"] = "Show this help message";
            
            m_commands["reset"] = [this](const auto& args) { return HandleReset(args); };
            m_commandDescriptions["reset"] = "Reset physics parameters to defaults";
        }
        
        std::vector<std::string> PhysicsConsole::ParseCommand(const std::string& command) {
            std::vector<std::string> tokens;
            std::istringstream iss(command);
            std::string token;
            
            while (iss >> token) {
                tokens.push_back(token);
            }
            
            return tokens;
        }
        
        // Debug drawing command implementations
        std::string PhysicsConsole::HandleEnableDebugDraw(const std::vector<std::string>& args) {
            m_engine->EnableDebugDrawing(true);
            return "Physics debug drawing enabled.";
        }
        
        std::string PhysicsConsole::HandleDisableDebugDraw(const std::vector<std::string>& args) {
            m_engine->EnableDebugDrawing(false);
            return "Physics debug drawing disabled.";
        }
        
        std::string PhysicsConsole::HandleSetDebugMode(const std::vector<std::string>& args) {
            if (args.size() < 2) {
                return "Usage: set_debug_mode <mode>\nModes: none, wireframe, aabb, contacts, all";
            }
            
            std::string mode = args[1];
            std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
            
            PhysicsDebugMode debugMode;
            if (mode == "none") {
                debugMode = PhysicsDebugMode::None;
            } else if (mode == "wireframe") {
                debugMode = PhysicsDebugMode::Wireframe;
            } else if (mode == "aabb") {
                debugMode = PhysicsDebugMode::AABB;
            } else if (mode == "contacts") {
                debugMode = PhysicsDebugMode::ContactPoints;
            } else if (mode == "all") {
                debugMode = PhysicsDebugMode::All;
            } else {
                return "Invalid debug mode: " + mode + "\nValid modes: none, wireframe, aabb, contacts, all";
            }
            
            m_engine->SetDebugMode(debugMode);
            return "Debug mode set to: " + mode;
        }
        
        std::string PhysicsConsole::HandleGetDebugMode(const std::vector<std::string>& args) {
            PhysicsDebugMode mode = m_engine->GetDebugMode();
            std::string modeStr;
            
            switch (mode) {
                case PhysicsDebugMode::None: modeStr = "none"; break;
                case PhysicsDebugMode::Wireframe: modeStr = "wireframe"; break;
                case PhysicsDebugMode::AABB: modeStr = "aabb"; break;
                case PhysicsDebugMode::ContactPoints: modeStr = "contacts"; break;
                case PhysicsDebugMode::All: modeStr = "all"; break;
                default: modeStr = "unknown"; break;
            }
            
            return "Current debug mode: " + modeStr;
        }
        
        // Physics parameter command implementations
        std::string PhysicsConsole::HandleSetGravity(const std::vector<std::string>& args) {
            if (args.size() < 4) {
                return "Usage: set_gravity <x> <y> <z>";
            }
            
            try {
                float x = std::stof(args[1]);
                float y = std::stof(args[2]);
                float z = std::stof(args[3]);
                
                m_engine->SetGravity(Math::Vec3(x, y, z));
                return "Gravity set to: " + FormatVec3(Math::Vec3(x, y, z));
            } catch (const std::exception& e) {
                return "Invalid gravity values. Usage: set_gravity <x> <y> <z>";
            }
        }
        
        std::string PhysicsConsole::HandleGetGravity(const std::vector<std::string>& args) {
            const auto& config = m_engine->GetConfiguration();
            return "Current gravity: " + FormatVec3(config.gravity);
        }
        
        std::string PhysicsConsole::HandleSetTimestep(const std::vector<std::string>& args) {
            if (args.size() < 2) {
                return "Usage: set_timestep <seconds>";
            }
            
            try {
                float timestep = std::stof(args[1]);
                if (timestep <= 0.0f || timestep > 1.0f) {
                    return "Timestep must be between 0 and 1 seconds.";
                }
                
                m_engine->SetTimeStep(timestep);
                return "Timestep set to: " + FormatFloat(timestep) + " seconds";
            } catch (const std::exception& e) {
                return "Invalid timestep value. Usage: set_timestep <seconds>";
            }
        }
        
        std::string PhysicsConsole::HandleGetTimestep(const std::vector<std::string>& args) {
            const auto& config = m_engine->GetConfiguration();
            return "Current timestep: " + FormatFloat(config.timeStep) + " seconds";
        }
        
        std::string PhysicsConsole::HandleSetSolverIterations(const std::vector<std::string>& args) {
            if (args.size() < 2) {
                return "Usage: set_solver_iterations <count>";
            }
            
            try {
                int iterations = std::stoi(args[1]);
                if (iterations < 1 || iterations > 100) {
                    return "Solver iterations must be between 1 and 100.";
                }
                
                m_engine->SetSolverIterations(iterations);
                return "Solver iterations set to: " + std::to_string(iterations);
            } catch (const std::exception& e) {
                return "Invalid iterations value. Usage: set_solver_iterations <count>";
            }
        }
        
        std::string PhysicsConsole::HandleGetSolverIterations(const std::vector<std::string>& args) {
            const auto& config = m_engine->GetConfiguration();
            return "Current solver iterations: " + std::to_string(config.solverIterations);
        }
        
        std::string PhysicsConsole::HandleSetContactThresholds(const std::vector<std::string>& args) {
            if (args.size() < 3) {
                return "Usage: set_contact_thresholds <breaking> <processing>";
            }
            
            try {
                float breaking = std::stof(args[1]);
                float processing = std::stof(args[2]);
                
                if (breaking <= 0.0f || processing <= 0.0f) {
                    return "Contact thresholds must be positive values.";
                }
                
                m_engine->SetContactThresholds(breaking, processing);
                return "Contact thresholds set to: breaking=" + FormatFloat(breaking) + 
                       ", processing=" + FormatFloat(processing);
            } catch (const std::exception& e) {
                return "Invalid threshold values. Usage: set_contact_thresholds <breaking> <processing>";
            }
        }
        
        std::string PhysicsConsole::HandleGetContactThresholds(const std::vector<std::string>& args) {
            const auto& config = m_engine->GetConfiguration();
            return "Contact thresholds: breaking=" + FormatFloat(config.contactBreakingThreshold) + 
                   ", processing=" + FormatFloat(config.contactProcessingThreshold);
        }
        
        // Information command implementations
        std::string PhysicsConsole::HandleDebugInfo(const std::vector<std::string>& args) {
            auto debugInfo = m_engine->GetDebugInfo();
            
            std::ostringstream oss;
            oss << "Physics Debug Information\n";
            oss << "========================\n";
            oss << "Rigid Bodies: " << debugInfo.numRigidBodies << "\n";
            oss << "Ghost Objects: " << debugInfo.numGhostObjects << "\n";
            oss << "Active Objects: " << debugInfo.numActiveObjects << "\n";
            oss << "Sleeping Objects: " << debugInfo.numSleepingObjects << "\n";
            oss << "Simulation Time: " << FormatFloat(debugInfo.simulationTime) << " ms\n";
            oss << "World Gravity: " << FormatVec3(debugInfo.worldGravity) << "\n";
            
            const auto& config = m_engine->GetConfiguration();
            oss << "\nConfiguration:\n";
            oss << "  Timestep: " << FormatFloat(config.timeStep) << " seconds\n";
            oss << "  Max Substeps: " << config.maxSubSteps << "\n";
            oss << "  Solver Iterations: " << config.solverIterations << "\n";
            oss << "  CCD Enabled: " << (config.enableCCD ? "Yes" : "No") << "\n";
            oss << "  Linear Damping: " << FormatFloat(config.linearDamping) << "\n";
            oss << "  Angular Damping: " << FormatFloat(config.angularDamping) << "\n";
            
            return oss.str();
        }
        
        std::string PhysicsConsole::HandleHelp(const std::vector<std::string>& args) {
            return GetHelpText();
        }
        
        std::string PhysicsConsole::HandleReset(const std::vector<std::string>& args) {
            auto defaultConfig = PhysicsConfiguration::Default();
            m_engine->SetConfiguration(defaultConfig);
            return "Physics parameters reset to defaults.";
        }
        
        // Utility functions
        std::string PhysicsConsole::FormatFloat(float value, int precision) const {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }
        
        std::string PhysicsConsole::FormatVec3(const Math::Vec3& vec, int precision) const {
            std::ostringstream oss;
            oss << "(" << FormatFloat(vec.x, precision) << ", " 
                << FormatFloat(vec.y, precision) << ", " 
                << FormatFloat(vec.z, precision) << ")";
            return oss.str();
        }
        
    } // namespace Physics
} // namespace GameEngine