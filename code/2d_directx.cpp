/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2017 by Barret Gaylor, Inc. All Rights Reserved. $
   ======================================================================== */
/*
  - work on 2d directx renderer
  - allow the system to use our own coordinate system
 */
#include <d3d11.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h>
//#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10.h>

IDXGISwapChain *Swapchain; 
ID3D11Device *DXDevice; 
ID3D11DeviceContext *DXDeviceContext;
ID3D11RenderTargetView *BackBuffer;
ID3D11VertexShader *VertexShader;    // the vertex shader
ID3D11PixelShader *PixelShader;     // the pixel shader
ID3D11Buffer *VertexBuffer;
ID3D11InputLayout *InputLayout; 
ID3D11Buffer *ConstantBuffer;
ID3D11Buffer *IndexBuffer;                // the pointer to the index buffer
ID3D11ShaderResourceView *Texture;    // the pointer to the texture

struct vertex
{
    v3 position;
    v3 normal; 
    v2 texcoord; 
};

struct cbuffer
{
    matrix4x4 Projection;
    v4 LightVector;
    v4 LightColor;
    v4 AmbientColor; 
};

void InitPipeline()
{
    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("2D_shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("2D_shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);
    
    DXDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &VertexShader);
    DXDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &PixelShader);

    // set the shader objects
    DXDeviceContext->VSSetShader(VertexShader, 0, 0);
    DXDeviceContext->PSSetShader(PixelShader, 0, 0);

    D3D11_INPUT_ELEMENT_DESC InputElementDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    DXDevice->CreateInputLayout(InputElementDesc, 3, VS->GetBufferPointer(), VS->GetBufferSize(),
                                &InputLayout);
    DXDeviceContext->IASetInputLayout(InputLayout);

    D3D11_BUFFER_DESC BufferDesc = {};

    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.ByteWidth = 176;
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DXDevice->CreateBuffer(&BufferDesc, 0, &ConstantBuffer);
    DXDeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
}

void InitGraphics(loaded_bitmap *Bitmap)
{
    // TODO(barret): this needs to be in my coordinate system
    // remember, each sprite can be made of 4 vertices and
    // 6 elements in an index buffer

    // TODO(barret): now i need to map a texture to a rectangle 

    
    vertex OurVertices[] =
        {
            {{200, 450, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},   // Index 0    Top-Left
            {{600, 150, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},   // Index 1    Bottom-Right
            {{200, 150, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},   // Index 2    Bottom-Left
            {{600, 450, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},   // Index 3    Top-Right
        };
   
    
    D3D11_BUFFER_DESC BufferDesc = {};

    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof(vertex) * 4;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DXDevice->CreateBuffer(&BufferDesc, 0, &VertexBuffer);

    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
    DXDeviceContext->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource);
    memcpy(MappedSubresource.pData, OurVertices, sizeof(OurVertices));
    DXDeviceContext->Unmap(VertexBuffer, 0);

    DWORD OurIndices[] =
        {
            0, 1, 2,
            3, 1, 0,
        };


    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof(DWORD) * 6;
    BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;

    DXDevice->CreateBuffer(&BufferDesc, 0, &IndexBuffer);

    DXDeviceContext->Map(IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource);
    memcpy(MappedSubresource.pData, OurIndices, sizeof(OurIndices));
    DXDeviceContext->Unmap(IndexBuffer, 0);

    /*
      D3DX11_IMAGE_LOAD_INFO ImageLoadInfo = {};
      ImageLoadInfo.Width = Bitmap->Width;
      ImageLoadInfo.Height = Bitmap->Height;
      ImageLoadInfo.Usage = D3D11_USAGE_DYNAMIC;
      ImageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;   
      ImageLoadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
      ImageLoadInfo.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
      ImageLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UINT;               
      ImageLoadInfo.MipFilter =  D3DX11_FILTER_LINEAR;

      D3DX11_IMAGE_INFO ImageInfo;
      D3DX11GetImageInfoFromMemory(Bitmap->Pixel, (Bitmap->Width * Bitmap->Height) * 4,
      0, &ImageInfo, 0);
    
      ImageLoadInfo.pSrcInfo = &ImageInfo; 
           
      HRESULT Result =  D3DX11CreateShaderResourceViewFromMemory(DXDevice,
      Bitmap->Pixel,
      (Bitmap->Width * Bitmap->Height) * 4,
      0, 0,
      &Texture,
      0);


      int i =1;
                                             
    
      // TODO(barret): D3DX11CreateShaderResourceViewFromFile has same effect 
      // also see if the problem is lighting
      */
    
    D3DX11CreateShaderResourceViewFromFile(DXDevice,            // the Direct3D device
                                           "Wood.png",    // load Wood.png in the local folder
                                           NULL,           // no additional information
                                           NULL,           // no multithreading
                                           &Texture,      // address of the shader-resource-view
                                           NULL);          // no multithreading

    
}

void InitDX2D(HWND Window, loaded_bitmap *Bitmap)
{
    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};

    SwapChainDesc.BufferCount = 1;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
    SwapChainDesc.BufferDesc.Height = SCREEN_HEIGHT; 
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.SampleDesc.Count = 4;
    SwapChainDesc.Windowed = 1;
    SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    D3D11CreateDeviceAndSwapChain(NULL,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  D3D11_SDK_VERSION,
                                  &SwapChainDesc,
                                  &Swapchain,
                                  &DXDevice,
                                  NULL,
                                  &DXDeviceContext);

    ID3D11Texture2D *TextureBackBuffer;
    Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&TextureBackBuffer);

    DXDevice->CreateRenderTargetView(TextureBackBuffer, 0, &BackBuffer);
    TextureBackBuffer->Release();

    DXDeviceContext->OMSetRenderTargets(1, &BackBuffer, 0);

    D3D11_VIEWPORT Viewport = {};

    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = SCREEN_WIDTH;
    Viewport.Height = SCREEN_HEIGHT;

    DXDeviceContext->RSSetViewports(1, &Viewport);

    InitPipeline();
    InitGraphics(Bitmap);
}

void RenderFrame(float dt)
{
    cbuffer CBuffer = {};
    
    float a = 2.0f/(float)SCREEN_WIDTH;
    float b = 2.0f/(float)SCREEN_HEIGHT;
    
    matrix4x4 Projection =
        {
            a, 0, 0, 0,
            0, b, 0, 0,
            0, 0, 1,  0,
            -1, -1, 0,  1
        };

    CBuffer.Projection = Projection;
    CBuffer.LightVector = {1.0f, 1.0f, 1.0f, 0.0f};
    CBuffer.LightColor = {0.5f, 0.5f, 0.5f, 1.0f};
    //CBuffer.AmbientColor = {1.0f, 1.0f, 1.0f, 1.0f};
    CBuffer.AmbientColor = {0.2f, 0.2f, 0.2f, 1.0f};

    DXDeviceContext->ClearRenderTargetView(BackBuffer, v4{0.0f, 0.2f, 0.4f, 1.0f}.E);

    unsigned int stride = sizeof(vertex);
    unsigned int offset = 0;
    DXDeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXDeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    DXDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    DXDeviceContext->UpdateSubresource(ConstantBuffer, 0, 0, &CBuffer, 0, 0);      
    DXDeviceContext->PSSetShaderResources(0, 1, &Texture);
    DXDeviceContext->DrawIndexed(6, 0, 0);

    Swapchain->Present(0, 0);
}

void CleanD3D()
{
    Swapchain->SetFullscreenState(0, 0);

    InputLayout->Release(); 
    VertexBuffer->Release();
    VertexShader->Release();
    PixelShader->Release();
    ConstantBuffer->Release();
    Swapchain->Release();
    BackBuffer->Release();
    DXDevice->Release();
    DXDeviceContext->Release();
    IndexBuffer->Release();
}
