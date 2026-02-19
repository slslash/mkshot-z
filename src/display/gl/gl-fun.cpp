/*
** gl-fun.cpp
**
** This file is part of mkxp, further modified for mkshot-z.
**
** mkxp is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "gl-fun.h"

#include "boost-hash.h"
#include "exception.h"

#include <SDL3/SDL_video.h>
#include <string>

GLFunctions gl;

typedef const GLubyte* (APIENTRYP _PFNGLGETSTRINGIPROC) (GLenum, GLuint);

static void parseExtensionsCore(_PFNGLGETINTEGERVPROC GetIntegerv, BoostSet<std::string> &out)
{
    _PFNGLGETSTRINGIPROC GetStringi =
    (_PFNGLGETSTRINGIPROC) SDL_GL_GetProcAddress("glGetStringi");
    
    GLint extCount = 0;
    GetIntegerv(GL_NUM_EXTENSIONS, &extCount);
    
    for (GLint i = 0; i < extCount; ++i)
        out.insert((const char*) GetStringi(GL_EXTENSIONS, i));
}

static void parseExtensionsCompat(_PFNGLGETSTRINGPROC GetString, BoostSet<std::string> &out)
{
    const char *ext = (const char*) GetString(GL_EXTENSIONS);
    
    if (!ext)
        return;
    
    char buffer[0x100];
    size_t bufferI;
    
    while (*ext)
    {
        bufferI = 0;
        while (*ext && *ext != ' ')
            buffer[bufferI++] = *ext++;
        
        buffer[bufferI] = '\0';
        
        out.insert(buffer);
        
        if (*ext == ' ')
            ++ext;
    }
}

#define GL_FUN(name, type) \
gl.name = (type) SDL_GL_GetProcAddress("gl" #name EXT_SUFFIX);

#define EXC(msg) \
Exception(Exception::MKXPError, "%s", msg)

void initGLFunctions()
{
#define EXT_SUFFIX ""
    GL_20_FUN;
    
    /* Determine GL version */
    const char *ver = (const char*) gl.GetString(GL_VERSION);
    
    const char glesPrefix[] = "OpenGL ES ";
    const size_t glesPrefixN = sizeof(glesPrefix)-1;
    
    bool gles = false;
    
    if (!strncmp(ver, glesPrefix, glesPrefixN))
    {
        gles = true;
        gl.glsles = true;
        
        ver += glesPrefixN;
    }
    
    /* Assume single digit */
    int glMajor = *ver - '0';
    
    if (glMajor < 2)
#ifndef GLES2_HEADER
        throw Exception(Exception::MKXPError,
                  "A graphics card that supports OpenGL 2.0 or later is required.\n\n"
                  "Driver information:\n"
                  "Vendor: %s\n"
                  "Renderer: %s\n"
                  "Version: %s\n"
                  "GLSL Version: %s\n",
                  gl.GetString(GL_VENDOR), gl.GetString(GL_RENDERER), gl.GetString(GL_VERSION),
                  gl.GetString(GL_SHADING_LANGUAGE_VERSION));
#else
        // on macOS, we're actually using either desktop GL or Metal due to ANGLE, but every Mac that supports Sierra
        // (officially or otherwise) should support ANGLE, so this should never be seen. Probably, anyway. Don't @ me
        throw EXC("A graphics card that supports OpenGL ES 2.0 or later is required.");
#endif
    
    if (gles)
    {
        GL_ES_FUN;
    }
    
    BoostSet<std::string> ext;
    
    if (glMajor >= 3)
        parseExtensionsCore(gl.GetIntegerv, ext);
    else
        parseExtensionsCompat(gl.GetString, ext);
    
#define HAVE_EXT(_ext) ext.contains("GL_" #_ext)
    
    /* FBO entrypoints */
    if (glMajor >= 3 || HAVE_EXT(ARB_framebuffer_object))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX ""
        GL_FBO_FUN;
        GL_FBO_BLIT_FUN;
    }
    else if (gles && glMajor == 2)
    {
        GL_FBO_FUN;
    }
    else if (HAVE_EXT(EXT_framebuffer_object))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX "EXT"
        GL_FBO_FUN;
        
        if (HAVE_EXT(EXT_framebuffer_blit))
        {
            GL_FBO_BLIT_FUN;
        }
    }
    else
    {
        throw EXC("No FBO support available");
    }
    
    /* VAO entrypoints */
    if (HAVE_EXT(ARB_vertex_array_object) || glMajor >= 3)
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX ""
        GL_VAO_FUN;
    }
    else if (HAVE_EXT(APPLE_vertex_array_object))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX "APPLE"
        GL_VAO_FUN;
    }
    else if (HAVE_EXT(OES_vertex_array_object))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX "OES"
        GL_VAO_FUN;
    }
    
    /* Debug callback entrypoints */
    if (HAVE_EXT(KHR_debug))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX ""
        GL_DEBUG_KHR_FUN;
    }
    else if (HAVE_EXT(ARB_debug_output))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX "ARB"
        GL_DEBUG_KHR_FUN;
    }
    
    if (HAVE_EXT(GREMEDY_string_marker))
    {
#undef EXT_SUFFIX
#define EXT_SUFFIX "GREMEDY"
        GL_GREMEMDY_FUN;
    }
    
    /* Misc caps */
    if (!gles || glMajor >= 3 || HAVE_EXT(EXT_unpack_subimage))
        gl.unpack_subimage = true;
    
    if (!gles || glMajor >= 3 || HAVE_EXT(OES_texture_npot))
        gl.npot_repeat = true;
}
