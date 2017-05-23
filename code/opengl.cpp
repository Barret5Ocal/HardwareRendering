/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2014 by Barret Gaylor. All Rights Reserved. $
   ======================================================================== */
#include <gl/gl.h>

#include "opengl_header.h"

#define GL_FRAMEBUFFER_SRGB                       0x8DB9
#define SRGB8_ALPHA8_EXT                          0x8C43

#define GL_SHADING_LANGUAGE_VERSION               0x8B8C

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_ACCELERATION_ARB                      0x2003

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

#define WGL_FRAMEBUFFER_SRBG_CAPABLE_ARB          0x20A9

struct opengl_info
{
    bool ModernContext;
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;
    char *Extensions;
    
    int GL_EXT_texture_sRGB;
    int GL_EXT_framebuffer_sRGB;
    int WGL_EXT_swap_control; 
};

opengl_info OpenGLGetInfo(bool ModernContext)
{
    opengl_info Extensions = {};
    
    Extensions.ModernContext = ModernContext; 
    
    Extensions.Vendor = (char *)glGetString(GL_VENDOR);
    Extensions.Renderer = (char *)glGetString(GL_RENDERER);
    Extensions.Version = (char *)glGetString(GL_VERSION);
    
    if(ModernContext)
        Extensions.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    else
        Extensions.ShadingLanguageVersion = "(none)";
    
    Extensions.Extensions = (char *)glGetString(GL_EXTENSIONS);
    
    char *At = Extensions.Extensions; 
    while(*At)
    {
        while(IsWhiteSpace(*At)){++At;}
        char *End = At;
        while (*End && !IsWhiteSpace(*End)){++End;}
        
        umm Count = End - At; 
        
        if(StringsAreEqual(Count, At, "GL_EXT_texture_sRGB")){Extensions.GL_EXT_texture_sRGB = true;}
        if(StringsAreEqual(Count, At, "GL_EXT_framebuffer_sRGB")){Extensions.GL_EXT_framebuffer_sRGB = true;}
        if(StringsAreEqual(Count, At, "WGL_EXT_swap_control")){Extensions.WGL_EXT_swap_control = true;}
        At = End;
    }
    
    return Extensions;
}


GLuint CompileShaders(render_fuctions *GLFuctions)
{
    static const char *VertexShaderSource[] =
    {
        "#version 450 core                                                      \n"
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

static unsigned int TextureHandleCount = 0;
static GLuint OpenGLDefaultTextureFormat;

static wgl_create_context_attribts_arb *wglCreateContextAttribsARB;
static wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
static wgl_swap_interval_ext *wglSwapInterval;

static void 
Win32SetPixelFormat(HDC WindowDC)
{
    int SuggestedPixelFormatIndex = 0;
    GLuint ExtendedPick = 0; 
    if(wglChoosePixelFormatARB)
    {
        int IntAttribList[] = 
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRBG_CAPABLE_ARB, GL_TRUE, 
            0
        };
        
        float FloatAttribList[] = {0};
        wglChoosePixelFormatARB(WindowDC, IntAttribList, FloatAttribList, 
                                1, &SuggestedPixelFormatIndex, &ExtendedPick);
        
    }
    
    if(!ExtendedPick)
    {
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        
        DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        
        DesiredPixelFormat.cColorBits = 32;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
        
        SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
        
    }
    
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
}

static void 
Win32LoadWGLExtensions()
{
    WNDCLASSA WindowClass = {};
    
    WindowClass.lpfnWndProc = DefWindowProc;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = "WGLLoader";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "OpenGL",
            0, 
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            WindowClass.hInstance,
            0);
        
        HDC WindowDC = GetDC(Window);
        Win32SetPixelFormat(WindowDC);
        HGLRC OpenGLRC = wglCreateContext(WindowDC);
        if(wglMakeCurrent(WindowDC, OpenGLRC))
        {
            wglCreateContextAttribsARB = 
                (wgl_create_context_attribts_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
            wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
            wglSwapInterval = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
            
            wglMakeCurrent(0, 0);
        }
        
        wglDeleteContext(OpenGLRC);
        ReleaseDC(Window, WindowDC);
        DestroyWindow(Window);
    }
}

int Win32OpenGLAttribs[] =
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    0,
};

HGLRC
Win32InitOpenGL(HWND Window, render_fuctions *Functions, GLuint *Program)
{
    Win32LoadWGLExtensions();
    HDC WindowDC = GetDC(Window);
    
    bool ModernContext = true;
    HGLRC OpenGLRC = 0;
    if(wglCreateContextAttribsARB)
    {
        Win32SetPixelFormat(WindowDC);
        OpenGLRC = wglCreateContextAttribsARB(WindowDC, 0, Win32OpenGLAttribs);
    }
    
    if(!OpenGLRC)
    {
        ModernContext = false;
        OpenGLRC = wglCreateContext(WindowDC);
    }
    
    if(wglMakeCurrent(WindowDC, OpenGLRC))
    {
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
        
        opengl_info Extensions = OpenGLGetInfo(ModernContext);
        
        
        OpenGLDefaultTextureFormat = GL_RGBA8;
        if(Extensions.GL_EXT_texture_sRGB)
        {
            OpenGLDefaultTextureFormat = SRGB8_ALPHA8_EXT; 
        }
        
        if(Extensions.GL_EXT_framebuffer_sRGB)
        {
            glEnable(GL_FRAMEBUFFER_SRGB);
        }
    }
    // NOTE(barret): YOU MUST CREATE A CONTEXT FIRST BEFORE GETTING WGL FUNCTIONS
    
    if(wglSwapInterval)
        wglSwapInterval(1);
    
    ReleaseDC(Window, WindowDC);
    return(OpenGLRC);
    /* 
       GLuint VertexArrayObject = 0;
       
       *Program = CompileShaders(Functions);
       Functions->glCreateVertexArrays(1, &VertexArrayObject);
       Functions->glBindVertexArray(VertexArrayObject);
    */    
}

void ClearScreen(v4 Color)
{
    //glDisable(GL_TEXTURE_2D);
    glClearColor(Color.E[0], Color.E[1], Color.E[2], Color.E[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    //glEnable(GL_TEXTURE_2D);
}

void DrawBitmap(loaded_bitmap *Bitmap, float X, float Y, v4 Color)
{
    if(Bitmap->Handle)
    {
        glBindTexture(GL_TEXTURE_2D, Bitmap->Handle);
    }
    else 
    {
        Bitmap->Handle = ++TextureHandleCount;
        glBindTexture(GL_TEXTURE_2D, Bitmap->Handle);
        
        glTexImage2D(GL_TEXTURE_2D, 0, OpenGLDefaultTextureFormat, Bitmap->Width, Bitmap->Height, 0,
                     GL_BGRA_EXT, GL_UNSIGNED_BYTE, Bitmap->Pixel);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    
    v2 MinP = {X, Y};
    v2 MaxP = {X + Bitmap->Width, Y + Bitmap->Height};
    
    glBegin(GL_TRIANGLES);
    
    glColor4f(Color.r, Color.g, Color.b, Color.a);
    
    // NOTE(casey): Lower triangle
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(MinP.x, MinP.y);
    
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(MaxP.x, MinP.y);
    
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(MaxP.x, MaxP.y);
    
    // NOTE(casey): Upper triangle
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(MinP.x, MinP.y);
    
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(MaxP.x, MaxP.y);
    
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(MinP.x, MaxP.y);
    
    glEnd();    
}

void RenderFrame(HDC DeviceContext, int WindowWidth, int WindowHeight,
                 render_fuctions *GLFuctions, render_data *Data)
{ 
    glViewport(0, 0, WindowWidth, WindowHeight);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // TODO(barret) figure out blend mode
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    // NOTE(barret): for screen coordinate projection 
    float a = 2.0f/(float)WindowWidth;
    float b = 2.0f/(float)WindowHeight;    
    matrix4x4 Projection =
    {
        a,  0, 0, 0,
        0,  b, 0, 0,
        0,  0, 1, 0,
        -1, -1, 0, 1
    };
    
    glLoadMatrixf(Projection.m); //NOTE(barret): for using our screen coordinates
    
    v4 Color = {1.0f, 0.0f,
        0.0f, 1.0f};
    
    ClearScreen(Color);
    
    v4 BitmapColor {1.0f, 1.0f, 1.0f, 1.0f};
    DrawBitmap(&Data->Texture, 0, 0, BitmapColor);
    
    /*
        //GLFuctions->glClearBufferfv(GL_COLOR, 0, Color);
        
        GLFuctions->glUseProgram(Data->Program); 
        
        GLFuctions->glDrawArrays(GL_TRIANGLES, 0, 3);
    */
    
    SwapBuffers(DeviceContext);
}
