#include "Input/InputManager.h"
#include "Core/Logger.h"
#include <GLFW/glfw3.h>

namespace GameEngine {
    InputManager::InputManager() {
    }

    InputManager::~InputManager() {
        Shutdown();
    }

    bool InputManager::Initialize(GLFWwindow* window) {
        m_window = window;
        
        // Set GLFW callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetScrollCallback(window, ScrollCallback);

        // Get initial mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        m_mousePosition = Math::Vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        m_lastMousePosition = m_mousePosition;

        // Capture mouse for 360° camera rotation
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        LOG_INFO("Input Manager initialized with mouse capture enabled");
        return true;
    }

    void InputManager::Shutdown() {
        if (m_window) {
            glfwSetKeyCallback(m_window, nullptr);
            glfwSetMouseButtonCallback(m_window, nullptr);
            glfwSetCursorPosCallback(m_window, nullptr);
            glfwSetScrollCallback(m_window, nullptr);
        }
        LOG_INFO("Input Manager shutdown");
    }

    void InputManager::Update() {
        UpdateKeyStates();
        UpdateMouseStates();
        UpdateGamepadStates();
        
        // Calculate mouse delta
        m_mouseDelta = m_mousePosition - m_lastMousePosition;
        m_lastMousePosition = m_mousePosition;
        
        // Reset scroll delta
        m_mouseScrollDelta = 0.0f;
    }

    bool InputManager::IsKeyPressed(KeyCode key) const {
        if (!m_window) return false;
        
        // For pressed detection, we need to track state changes
        // For now, let's use a simple approach - check if key is currently down
        // This isn't perfect for single-press detection but will work for jump
        static std::unordered_map<int, bool> lastKeyState;
        int keyCode = static_cast<int>(key);
        bool currentState = glfwGetKey(m_window, keyCode) == GLFW_PRESS;
        bool wasPressed = currentState && !lastKeyState[keyCode];
        lastKeyState[keyCode] = currentState;
        return wasPressed;
    }

    bool InputManager::IsKeyDown(KeyCode key) const {
        if (!m_window) return false;
        return glfwGetKey(m_window, static_cast<int>(key)) == GLFW_PRESS;
    }

    bool InputManager::IsKeyReleased(KeyCode key) const {
        auto it = m_keyStates.find(static_cast<int>(key));
        return it != m_keyStates.end() && it->second.wasReleased;
    }

    bool InputManager::IsMouseButtonPressed(MouseButton button) const {
        auto it = m_mouseButtonStates.find(static_cast<int>(button));
        return it != m_mouseButtonStates.end() && it->second.wasPressed;
    }

    bool InputManager::IsMouseButtonDown(MouseButton button) const {
        auto it = m_mouseButtonStates.find(static_cast<int>(button));
        return it != m_mouseButtonStates.end() && it->second.isPressed;
    }

    bool InputManager::IsMouseButtonReleased(MouseButton button) const {
        auto it = m_mouseButtonStates.find(static_cast<int>(button));
        return it != m_mouseButtonStates.end() && it->second.wasReleased;
    }

    Math::Vec2 InputManager::GetMousePosition() const {
        return m_mousePosition;
    }

    Math::Vec2 InputManager::GetMouseDelta() const {
        return m_mouseDelta;
    }

    float InputManager::GetMouseScrollDelta() const {
        return m_mouseScrollDelta;
    }

    bool InputManager::IsGamepadConnected(int gamepadId) const {
        return glfwJoystickPresent(gamepadId) == GLFW_TRUE;
    }

    bool InputManager::IsGamepadButtonPressed(GamepadButton button, int gamepadId) const {
        auto gamepadIt = m_gamepadButtonStates.find(gamepadId);
        if (gamepadIt == m_gamepadButtonStates.end()) return false;
        
        auto buttonIt = gamepadIt->second.find(static_cast<int>(button));
        return buttonIt != gamepadIt->second.end() && buttonIt->second.wasPressed;
    }

    bool InputManager::IsGamepadButtonDown(GamepadButton button, int gamepadId) const {
        auto gamepadIt = m_gamepadButtonStates.find(gamepadId);
        if (gamepadIt == m_gamepadButtonStates.end()) return false;
        
        auto buttonIt = gamepadIt->second.find(static_cast<int>(button));
        return buttonIt != gamepadIt->second.end() && buttonIt->second.isPressed;
    }

    bool InputManager::IsGamepadButtonReleased(GamepadButton button, int gamepadId) const {
        auto gamepadIt = m_gamepadButtonStates.find(gamepadId);
        if (gamepadIt == m_gamepadButtonStates.end()) return false;
        
        auto buttonIt = gamepadIt->second.find(static_cast<int>(button));
        return buttonIt != gamepadIt->second.end() && buttonIt->second.wasReleased;
    }

    float InputManager::GetGamepadAxis(GamepadAxis axis, int gamepadId) const {
        auto gamepadIt = m_gamepadAxisStates.find(gamepadId);
        if (gamepadIt == m_gamepadAxisStates.end()) return 0.0f;
        
        auto axisIt = gamepadIt->second.find(static_cast<int>(axis));
        return axisIt != gamepadIt->second.end() ? axisIt->second : 0.0f;
    }

    void InputManager::BindAction(const std::string& actionName, KeyCode key) {
        ActionBinding binding;
        binding.type = ActionBinding::Key;
        binding.code = static_cast<int>(key);
        m_actionBindings[actionName].push_back(binding);
    }

    void InputManager::BindAction(const std::string& actionName, MouseButton button) {
        ActionBinding binding;
        binding.type = ActionBinding::MouseButton;
        binding.code = static_cast<int>(button);
        m_actionBindings[actionName].push_back(binding);
    }

    void InputManager::BindAction(const std::string& actionName, GamepadButton button, int gamepadId) {
        ActionBinding binding;
        binding.type = ActionBinding::GamepadButton;
        binding.code = static_cast<int>(button);
        binding.gamepadId = gamepadId;
        m_actionBindings[actionName].push_back(binding);
    }

    bool InputManager::IsActionPressed(const std::string& actionName) const {
        auto it = m_actionBindings.find(actionName);
        if (it == m_actionBindings.end()) return false;

        for (const auto& binding : it->second) {
            switch (binding.type) {
                case ActionBinding::Key:
                    if (IsKeyPressed(static_cast<KeyCode>(binding.code))) return true;
                    break;
                case ActionBinding::MouseButton:
                    if (IsMouseButtonPressed(static_cast<MouseButton>(binding.code))) return true;
                    break;
                case ActionBinding::GamepadButton:
                    if (IsGamepadButtonPressed(static_cast<GamepadButton>(binding.code), binding.gamepadId)) return true;
                    break;
            }
        }
        return false;
    }

    bool InputManager::IsActionDown(const std::string& actionName) const {
        auto it = m_actionBindings.find(actionName);
        if (it == m_actionBindings.end()) return false;

        for (const auto& binding : it->second) {
            switch (binding.type) {
                case ActionBinding::Key:
                    if (IsKeyDown(static_cast<KeyCode>(binding.code))) return true;
                    break;
                case ActionBinding::MouseButton:
                    if (IsMouseButtonDown(static_cast<MouseButton>(binding.code))) return true;
                    break;
                case ActionBinding::GamepadButton:
                    if (IsGamepadButtonDown(static_cast<GamepadButton>(binding.code), binding.gamepadId)) return true;
                    break;
            }
        }
        return false;
    }

    bool InputManager::IsActionReleased(const std::string& actionName) const {
        auto it = m_actionBindings.find(actionName);
        if (it == m_actionBindings.end()) return false;

        for (const auto& binding : it->second) {
            switch (binding.type) {
                case ActionBinding::Key:
                    if (IsKeyReleased(static_cast<KeyCode>(binding.code))) return true;
                    break;
                case ActionBinding::MouseButton:
                    if (IsMouseButtonReleased(static_cast<MouseButton>(binding.code))) return true;
                    break;
                case ActionBinding::GamepadButton:
                    if (IsGamepadButtonReleased(static_cast<GamepadButton>(binding.code), binding.gamepadId)) return true;
                    break;
            }
        }
        return false;
    }

    void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (inputManager && inputManager->m_keyCallback) {
            inputManager->m_keyCallback(static_cast<KeyCode>(key), action != GLFW_RELEASE);
        }
    }

    void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (inputManager && inputManager->m_mouseButtonCallback) {
            inputManager->m_mouseButtonCallback(static_cast<MouseButton>(button), action != GLFW_RELEASE);
        }
    }

    void InputManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (inputManager) {
            inputManager->m_mousePosition = Math::Vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    }

    void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (inputManager) {
            inputManager->m_mouseScrollDelta = static_cast<float>(yoffset);
        }
    }

    void InputManager::UpdateKeyStates() {
        for (auto& [key, state] : m_keyStates) {
            state.wasPressed = false;
            state.wasReleased = false;
            
            bool currentPressed = glfwGetKey(m_window, key) == GLFW_PRESS;
            if (currentPressed && !state.isPressed) {
                state.wasPressed = true;
            } else if (!currentPressed && state.isPressed) {
                state.wasReleased = true;
            }
            state.isPressed = currentPressed;
        }
    }

    void InputManager::UpdateMouseStates() {
        for (auto& [button, state] : m_mouseButtonStates) {
            state.wasPressed = false;
            state.wasReleased = false;
            
            bool currentPressed = glfwGetMouseButton(m_window, button) == GLFW_PRESS;
            if (currentPressed && !state.isPressed) {
                state.wasPressed = true;
            } else if (!currentPressed && state.isPressed) {
                state.wasReleased = true;
            }
            state.isPressed = currentPressed;
        }
    }

    void InputManager::UpdateGamepadStates() {
        for (int gamepadId = 0; gamepadId < 4; ++gamepadId) {
            if (!IsGamepadConnected(gamepadId)) continue;

            GLFWgamepadstate state;
            if (glfwGetGamepadState(gamepadId, &state)) {
                // Update button states
                auto& buttonStates = m_gamepadButtonStates[gamepadId];
                for (int i = 0; i < 15; ++i) {
                    auto& buttonState = buttonStates[i];
                    buttonState.wasPressed = false;
                    buttonState.wasReleased = false;
                    
                    bool currentPressed = state.buttons[i] == GLFW_PRESS;
                    if (currentPressed && !buttonState.isPressed) {
                        buttonState.wasPressed = true;
                    } else if (!currentPressed && buttonState.isPressed) {
                        buttonState.wasReleased = true;
                    }
                    buttonState.isPressed = currentPressed;
                }

                // Update axis states
                auto& axisStates = m_gamepadAxisStates[gamepadId];
                for (int i = 0; i < 6; ++i) {
                    axisStates[i] = state.axes[i];
                }
            }
        }
    }
}