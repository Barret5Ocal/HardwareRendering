/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2017 by Barret Gaylor, Inc. All Rights Reserved. $
   ======================================================================== */
#include <windows.h>
#include <stdio.h>
#include "timing.h"
#include <math.h>
#include <stdint.h>

#include "load_image.h"

typedef uint32_t uint32;
typedef int32_t int32; 

static bool Running = 1; 

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

typedef float r32;

union v2
{
    struct
    {
        float x;
        float y;
    };
    struct
    {
        float u;
        float v;
    };
    float E[2];
};

union v3
{
    struct
    {
        float x;
        float y;
        float z; 
    };
    float E[3]; 
};

union v4
{
    struct
    {
        float x;
        float y;
        float z;
        float w;
    };
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
    struct
    {
        v3 xyz;
        float w;
    };
    float E[4];
};

union matrix4x4
{
    struct
    {
        float        _11, _12, _13, _14;
        float        _21, _22, _23, _24;
        float        _31, _32, _33, _34;
        float        _41, _42, _43, _44;

    };
    float m[4][4];
};

inline v3
V3Transform(v3 Vec, matrix4x4 Mat)
{
    float x = (Vec.x * Mat._11) + (Vec.y * Mat._12) + (Vec.z * Mat._13) + (1.0f* Mat._14);
    float y = (Vec.x * Mat._21) + (Vec.y * Mat._22) + (Vec.z * Mat._23) + (1.0f * Mat._24);
    float z = (Vec.x * Mat._31) + (Vec.y * Mat._32) + (Vec.z * Mat._33) + (1.0f * Mat._34);
    float w = (Vec.x * Mat._41) + (Vec.y * Mat._42) + (Vec.z * Mat._43) + (1.0f * Mat._44);

    v3 Result = {x, y, z};
    return Result; 
}

// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

#if OPENGL
#include "opengl.cpp"
#endif

#if VULKAN
#include "vulkan.cpp"
#endif

#if DIRECTX
#include "2D_directx.cpp"
#endif

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // sort through and find what code to run for the message given
    switch(message)
    {
        // this message is read when the window is closed
        case WM_DESTROY:
        {
            // close the application entirely
            PostQuitMessage(0);
            Running = 0;
            return 0;
        } break;
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            Running = 0;
            return 0;            
        }break; 
    }

    // Handle any messages the switch statement didn't
    return DefWindowProc (hWnd, message, wParam, lParam);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CmdLine,
        int ShowCode)
{   

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "WindowClass";
    WindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    
    if(RegisterClass(&WindowClass))
    {
        RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    
        HWND Window = CreateWindow(WindowClass.lpszClassName,
                                   "HardwareRendering",
                                   WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   wr.right - wr.left,
                                   wr.bottom - wr.top,                                   
                                   0,
                                   0,
                                   Instance,
                                   0);

        render_fuctions Functions = {};
        render_data Data = {};

        loaded_bitmap Bitmap = LoadImage("Wood.png");
        
#if OPENGL
        GLuint Program = 0;
        Win32InitOpenGL(Window, &Functions, &Program);
        Data.Program = Program; 
#endif
#if VULKAN
        Win32InitVulkan(Instance, Window);
#endif
#if DIRECTX
        InitDX2D(Window, &Bitmap);
        //InitD3D(Window);
#endif
        
        RECT Rect = {};
        GetClientRect(Window, &Rect);
        int WindowWidth = Rect.right - Rect.left;
        int WindowHeight = Rect.bottom - Rect.top;

        float GameUpdateHz = 60;        
        float TargetSecondsPerFrame = 1.0f / (float)GameUpdateHz;
        
        time_info TimeInfo = {};
        while(RunLoop(&TimeInfo, Running, GameUpdateHz))
        {
            MSG Message;
            while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
            
            HDC DeviceContext = GetDC(Window);
#if DIRECTX
            RenderFrame(TargetSecondsPerFrame);
#endif
#if OPENGL
            RenderFrame(DeviceContext, WindowWidth, WindowHeight,
                        &Functions, &Data);
#endif
            ReleaseDC(Window, DeviceContext);

        }
        
    }

#if DIRECTX
    CleanD3D();
#endif 
    return 0; 
}
