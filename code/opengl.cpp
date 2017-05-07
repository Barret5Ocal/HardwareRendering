/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2014 by Barret Gaylor. All Rights Reserved. $
   ======================================================================== */
#include <gl/gl.h>

#include "opengl_header.h"

static bool Running = 1;

GLuint CompileShaders(render_fuctions *GLFuctions)
{
    static const char *VertexShaderSource[] =
        {
            "#version 450 core                                                  \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    const vec4 Vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0), \n"
            "                                     vec4(-0.25, -0.25, 0.5, 1.0), \n"
            "                                     vec4( 0.25,  0.25, 0.5, 1.0));\n"
            "                                                                   \n"
            "    gl_Position = Vertices[gl_VertexID];                           \n"
            "}                                                                  \n"
        };

    static const char *FragmentShaderSource[] =
        {
            "#version 450 core                            \n"
            "                                             \n"
            "out vec4 Color;                              \n"
            "                                             \n"
            "void main(void)                              \n"
            "{                                            \n"
            "    color = vec4(0.0, 0.8, 1.0, 1.0);        \n"
            "}                                            \n"
        };

    GLuint VertexShader = GLFuctions->glCreateShader(GL_VERTEX_SHADER);
    GLFuctions->glShaderSource(VertexShader, 1, VertexShaderSource, 0);
    GLFuctions->glCompileShader(VertexShader);

    GLuint FragmentShader = GLFuctions->glCreateShader(GL_FRAGMENT_SHADER);
    GLFuctions->glShaderSource(FragmentShader, 1, FragmentShaderSource, 0);
    GLFuctions->glCompileShader(FragmentShader);

    GLuint Program = GLFuctions->glCreateProgram();
    GLFuctions->glAttachShader(Program, VertexShader);
    GLFuctions->glAttachShader(Program, FragmentShader);
    GLFuctions->glLinkProgram(Program);

    GLFuctions->glDeleteShader(VertexShader);
    GLFuctions->glDeleteShader(FragmentShader);
    
    return(Program);
        
}

void
Win32InitOpenGL(HWND Window, render_fuctions *Functions, GLuint *Program)
{
    HDC WindowDC = GetDC(Window);

    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                       sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    
    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    if(wglMakeCurrent(WindowDC, OpenGLRC))
    {
        
    }
    else
    {
        Assert(false);
    }
    
    ReleaseDC(Window, WindowDC);
        
        
    Functions->glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)GetAnyGLFuncAddress("glClearBufferfv");
    Functions->glCreateShader = (PFNGLCREATESHADERPROC)GetAnyGLFuncAddress("glCreateShader");
    Functions->glShaderSource = (PFNGLSHADERSOURCEPROC)GetAnyGLFuncAddress("glShaderSource");
    Functions->glCompileShader = (PFNGLCOMPILESHADERPROC)GetAnyGLFuncAddress("glCompileShader");    
    Functions->glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetAnyGLFuncAddress("glCreateProgram");
    Functions->glAttachShader = (PFNGLATTACHSHADERPROC)GetAnyGLFuncAddress("glAttachShader"); 
    Functions->glLinkProgram = (PFNGLLINKPROGRAMPROC)GetAnyGLFuncAddress("glLinkProgram");
    Functions->glDeleteShader = (PFNGLDELETESHADERPROC)GetAnyGLFuncAddress("glDeleteShader");         
    Functions->glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)GetAnyGLFuncAddress("glCreateVertexArrays");
    Functions->glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetAnyGLFuncAddress("glBindVertexArray");
    Functions->glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) GetAnyGLFuncAddress("glDeleteVertexArrays");
    Functions->glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GetAnyGLFuncAddress("glDeleteProgram");
    Functions->glUseProgram = (PFNGLUSEPROGRAMPROC)GetAnyGLFuncAddress("glUseProgram");      
    Functions->glDrawArrays = (PFNGLDRAWARRAYSEXTPROC)GetAnyGLFuncAddress("glDrawArrays");
    
    GLuint VertexArrayObject = 0;
        
    *Program = CompileShaders(Functions);
    Functions->glCreateVertexArrays(1, &VertexArrayObject);
    Functions->glBindVertexArray(VertexArrayObject);
    
}

void DisplayFrame(HDC DeviceContext, int WindowWidth, int WindowHeight,
                  render_fuctions *GLFuctions, render_data *Data)
{
    glViewport(0, 0, WindowWidth, WindowHeight);
    

    GLfloat Color[] = {0.0f,
                       0.2f,
                       0.0f, 1.0f};

    GLFuctions->glClearBufferfv(GL_COLOR, 0, Color);

    GLFuctions->glUseProgram(Program); 

    GLFuctions->glDrawArrays(GL_TRIANGLES, 0, 3);
    
    SwapBuffers(DeviceContext);
}
