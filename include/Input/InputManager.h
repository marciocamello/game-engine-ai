#pragma once

#include "../../engine/core/Math.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace GameEngine {
    enum class KeyCode {
        // Letters
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        
        // Numbers
        Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        
        // Function keys
        F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        
        // Special keys
        Space = 32,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Delete = 261,
        Escape = 256,
        LeftShift = 340,
        RightShift = 344,
        LeftCtrl = 341,
        RightCtrl = 345,
        LeftAlt = 342,
        RightAlt = 346,
        
        // Arrow keys
        Up = 265,
        Down = 264,
        Left = 263,
        Right = 262
    };

    enum class MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2
    };

    enum class GamepadButton {
        A = 0, B, X, Y,
        LeftBumper, RightBumper,
        Back, Start, Guide,
        LeftThumb, RightThumb,
        DPadUp, DPadRight, DPadDown, DPadLeft
    };

    enum class GamepadAxis {
        LeftX = 0, LeftY,
        RightX, RightY,
        LeftTrigger, RightTrigger
    };

    struct InputState {
        bool isPressed = false;
        bool wasPressed = false;
        bool wasReleased = false;
    };

    class InputManager {
    public:
        InputManager();
        ~InputManager();

        bool Initialize(GLFWwindow* window);
        void Shutdown();
        void Update();

        // Keyboard input
        bool IsKeyPressed(KeyCode key) const;
        bool IsKeyDown(KeyCode key) const;
        bool IsKeyReleased(KeyCode key) const;

        // Mouse input
        bool IsMouseButtonPressed(MouseButton button) const;
        bool IsMouseButtonDown(MouseButton button) const;
        bool IsMouseButtonReleased(MouseButton button) const;
        Math::Vec2 GetMousePosition() const;
        Math::Vec2 GetMouseDelta() const;
        float GetMouseScrollDelta() const;

        // Gamepad input
        bool IsGamepadConnected(int gamepadId = 0) const;
        bool IsGamepadButtonPressed(GamepadButton button, int gamepadId = 0) const;
        bool IsGamepadButtonDown(GamepadButton button, int gamepadId = 0) const;
        bool IsGamepadButtonReleased(GamepadButton button, int gamepadId = 0) const;
        float GetGamepadAxis(GamepadAxis axis, int gamepadId = 0) const;

        // Input binding system
        void BindAction(const std::string& actionName, KeyCode key);
        void BindAction(const std::string& actionName, MouseButton button);
        void BindAction(const std::string& actionName, GamepadButton button, int gamepadId = 0);
        bool IsActionPressed(const std::string& actionName) const;
        bool IsActionDown(const std::string& actionName) const;
        bool IsActionReleased(const std::string& actionName) const;

        // Callbacks
        void SetKeyCallback(std::function<void(KeyCode, bool)> callback) { m_keyCallback = callback; }
        void SetMouseButtonCallback(std::function<void(MouseButton, bool)> callback) { m_mouseButtonCallback = callback; }

    private:
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        void UpdateKeyStates();
        void UpdateMouseStates();
        void UpdateGamepadStates();

        GLFWwindow* m_window = nullptr;
        
        std::unordered_map<int, InputState> m_keyStates;
        std::unordered_map<int, InputState> m_mouseButtonStates;
        std::unordered_map<int, std::unordered_map<int, InputState>> m_gamepadButtonStates;
        std::unordered_map<int, std::unordered_map<int, float>> m_gamepadAxisStates;

        Math::Vec2 m_mousePosition{0.0f};
        Math::Vec2 m_lastMousePosition{0.0f};
        Math::Vec2 m_mouseDelta{0.0f};
        float m_mouseScrollDelta = 0.0f;

        // Input bindings
        struct ActionBinding {
            enum Type { Key, MouseButton, GamepadButton } type;
            int code;
            int gamepadId = 0;
        };
        std::unordered_map<std::string, std::vector<ActionBinding>> m_actionBindings;

        std::function<void(KeyCode, bool)> m_keyCallback;
        std::function<void(MouseButton, bool)> m_mouseButtonCallback;
    };
}