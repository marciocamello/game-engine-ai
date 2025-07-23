#pragma once

namespace GameEngine {
    /**
     * Utility class for OpenGL context management and checking
     */
    class OpenGLContext {
    public:
        /**
         * Check if there is an active OpenGL context
         * @return true if OpenGL context is available, false otherwise
         */
        static bool HasActiveContext();
        
        /**
         * Check if OpenGL is initialized and ready for use
         * @return true if OpenGL is ready, false otherwise
         */
        static bool IsReady();
        
        /**
         * Get OpenGL version string (if context is available)
         * @return OpenGL version string or empty string if no context
         */
        static const char* GetVersionString();
        
    private:
        OpenGLContext() = default;
    };
}