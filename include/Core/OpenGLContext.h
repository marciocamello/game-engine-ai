#pragma once

namespace GameEngine {
    /**
     * @brief Utility class for OpenGL context management and checking
     */
    class OpenGLContext {
    public:
        /**
         * @brief Check if there is an active OpenGL context
         * @return true if OpenGL context is available, false otherwise
         */
        static bool HasActiveContext();
        
        /**
         * @brief Check if OpenGL is initialized and ready for use
         * @return true if OpenGL is ready, false otherwise
         */
        static bool IsReady();
        
        /**
         * @brief Get OpenGL version string (for debugging)
         * @return OpenGL version string or empty if no context
         */
        static const char* GetVersionString();
        
    private:
        OpenGLContext() = delete; // Static utility class
    };
}