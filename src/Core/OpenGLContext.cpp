#include "Core/OpenGLContext.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace GameEngine {
    
    bool OpenGLContext::HasActiveContext() {
        // First check if GLAD is loaded
        if (!gladLoadGL()) {
            return false;
        }
        
        // Try to check if GLAD has been initialized
        try {
            // Try to get OpenGL version - this will fail if no context is active
            const GLubyte* version = glGetString(GL_VERSION);
            
            // Check for OpenGL errors
            GLenum error = glGetError();
            
            // If we got a version string and no error, context is active
            return (version != nullptr && error == GL_NO_ERROR);
        } catch (...) {
            // If any exception occurs, assume no context
            return false;
        }
    }
    
    bool OpenGLContext::IsReady() {
        // Check if context exists and GLAD is initialized
        if (!HasActiveContext()) {
            return false;
        }
        
        // Additional check: try to get renderer info
        const GLubyte* renderer = glGetString(GL_RENDERER);
        GLenum error = glGetError();
        
        return (renderer != nullptr && error == GL_NO_ERROR);
    }
    
    const char* OpenGLContext::GetVersionString() {
        if (!HasActiveContext()) {
            return "";
        }
        
        const GLubyte* version = glGetString(GL_VERSION);
        return version ? reinterpret_cast<const char*>(version) : "";
    }
}