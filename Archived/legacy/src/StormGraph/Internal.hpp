#pragma once

#include <SDL_ttf.h>

#define __StormGraph_Internal__
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    extern bool shadersEnabled;

    /*
      when adding a dynamically linked GL function:
        - add its prototype/definition to GlFunctionTable (ordered alphabetically)
        - add its official name to glFunctionNames[] (ordered alphabetically)
        - !!! don't forget to adjust the size of GlFunctionTable.functions[] !!!
     */

#ifdef __StormGraph_Engine_cpp__
    bool shadersEnabled;

    enum
    {
        Gl_shaders = 1,
        Gl_optional = 2
    };

    struct GlFunction
    {
        const char* name;
        unsigned flags;
    };

    static GlFunction glLinkTable[] =
    {
        { "glAttachShader", Gl_shaders },
        { "glBindBuffer", 0 },
        { "glBlendEquationSeparate", Gl_optional },
        { "glBufferData", 0 },
        { "glCompileShader", Gl_shaders },
        { "glCreateProgram", Gl_shaders },
        { "glCreateShader", Gl_shaders },
        { "glDeleteBuffers", 0 },
        { "glDeleteProgram", Gl_shaders },
        { "glDeleteShader", Gl_shaders },
        { "glDetachShader", Gl_shaders },
        { "glGenBuffers", 0 },
        { "glGetAttribLocation", Gl_shaders },
        { "glGetProgramiv", Gl_shaders },
        { "glGetProgramInfoLog", Gl_shaders },
        { "glGetShaderiv", Gl_shaders },
        { "glGetShaderInfoLog", Gl_shaders },
        { "glGetUniformLocation", Gl_shaders },
        { "glLinkProgram", Gl_shaders },
        { "glShaderSource", Gl_shaders },
        { "glUniform1f", Gl_shaders },
        { "glUniform3f", Gl_shaders },
        { "glUniform4f", Gl_shaders },
        { "glUniformMatrix4fv", Gl_shaders },
        { "glUseProgram", Gl_shaders },
        { "glVertexAttrib4f", Gl_shaders }
    };
#endif

    extern union GlFunctionTable
    {
        struct
        {
            PFNGLATTACHSHADERPROC AttachShader;
            PFNGLBINDBUFFERPROC glBindBuffer;
            PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
            PFNGLBUFFERDATAPROC glBufferData;
            PFNGLCOMPILESHADERPROC CompileShader;
            PFNGLCREATEPROGRAMPROC CreateProgram;
            PFNGLCREATESHADERPROC CreateShader;
            PFNGLDELETEBUFFERSPROC glDeleteBuffers;
            PFNGLDELETEPROGRAMPROC DeleteProgram;
            PFNGLDELETESHADERPROC DeleteShader;
            PFNGLDETACHSHADERPROC DetachShader;
            PFNGLGENBUFFERSPROC glGenBuffers;
            PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
            PFNGLGETPROGRAMIVPROC GetProgramiv;
            PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
            PFNGLGETSHADERIVPROC GetShaderiv;
            PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
            PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
            PFNGLLINKPROGRAMPROC LinkProgram;
            PFNGLSHADERSOURCEPROC ShaderSource;
            PFNGLUNIFORM1FPROC glUniform1f;
            PFNGLUNIFORM3FPROC glUniform3f;
            PFNGLUNIFORM4FPROC glUniform4f;
            PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
            PFNGLUSEPROGRAMPROC UseProgram;
            PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
        };

        void* functions[26];
    }
    glFs;

    void loadMs3d( List<Mesh*>& meshes, InputStream* input, ResourceManager* resMgr );

    SDL_RWops* getRwOps( SeekableInputStream* stream );
}
