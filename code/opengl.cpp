/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2014 by Barret Gaylor. All Rights Reserved. $
   ======================================================================== */
#include <gl/gl.h>

#include "opengl_header.h"


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

GLuint GlobalBlitTextureHandle;

void
Win32InitOpenGL(HWND Window, render_fuctions *Functions, GLuint *Program)
{
    HDC WindowDC = GetDC(Window);

    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
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
        glGenTextures(1, &GlobalBlitTextureHandle);
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
    /* 
    GLuint VertexArrayObject = 0;
        
    *Program = CompileShaders(Functions);
    Functions->glCreateVertexArrays(1, &VertexArrayObject);
    Functions->glBindVertexArray(VertexArrayObject);
    */    
}

void RenderFrame(HDC DeviceContext, int WindowWidth, int WindowHeight,
                  render_fuctions *GLFuctions, render_data *Data)
{
    glViewport(0, 0, WindowWidth, WindowHeight);
    

    GLfloat Color[] = {0.0f,
                       0.2f,
                       0.0f, 1.0f};

    glBindTexture(GL_TEXTURE_2D, GlobalBlitTextureHandle);

    loaded_bitmap Bitmap = Data->Texture;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Bitmap.Width, Bitmap.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 Bitmap.Pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);

    glClearColor(Color[0], Color[1], Color[2], Color[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glBegin(GL_TRIANGLES);

    float P = 1.0f;

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-P, -P);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(P, -P);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(P, P);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-P, -P);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(P, P);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-P, P);

    glEnd();
    //GLFuctions->glClearBufferfv(GL_COLOR, 0, Color);
/*
    GLFuctions->glUseProgram(Data->Program); 

    GLFuctions->glDrawArrays(GL_TRIANGLES, 0, 3);
*/    
    SwapBuffers(DeviceContext);
}
