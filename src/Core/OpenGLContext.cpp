#include "Core/OpenGLContext.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace GameEngine {
    
    bool OpenGLContext::HasActiveContext() {
        // Try to get OpenGL version - this will fail if no context is active
        const GLubyte* version = glGetString(GL_VERSION);
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        
        // If we got a version string and no error, context is active
        return (version != nullptr && error == GL_NO_ERROR);
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