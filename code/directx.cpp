/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2017 by Barret Gaylor, Inc. All Rights Reserved. $
   ======================================================================== */

#include <d3d11.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10.h>

// global declarations
IDXGISwapChain *Swapchain;             // the pointer to the swap chain interface
ID3D11Device *DXDevice;                // the pointer to our Direct3D device interface
ID3D11DeviceContext *DXDeviceContext;   // the pointer to our Direct3D device context
ID3D11RenderTargetView *BackBuffer;
ID3D11DepthStencilView *ZBuffer;       // the pointer to our depth buffer
ID3D11InputLayout *InputLayout;     // the pointer to the input layout   
ID3D11VertexShader *VertexShader;   // the vertex shader
ID3D11PixelShader *PixelShader;     // the pixel shader
ID3D11Buffer *VertexBuffer;                // the pointer to the vertex buffer
ID3D11Buffer *IndexBuffer;          // the pointer to the index buffer
ID3D11Buffer *ConstantBuffer;                // the pointer to the constant buffer
ID3D11ShaderResourceView *Texture;    // the pointer to the texture

// state objects
ID3D11RasterizerState *RasterState;     // the default rasterizer state
ID3D11SamplerState *SamplerStates;   // three sampler states
ID3D11BlendState *BlendState;                 // a typicl blend state

// a struct to define a single vertex
struct vertex {v3 Position; v3 Normal; v2 UV;};

// a struct to define the constant buffer
struct cbuffer
{
    D3DXMATRIX Final;
    D3DXMATRIX Rotation;
    v4 LightVector;
    v4 LightColor;
    v4 AmbientColor;
};

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory
void InitGraphics(void);    // creates the shape to render
void InitPipeline(void);    // loads and prepares the shaders
void InitStates(void);      // initializes the states

// this function initializes and prepares Direct3D for use
void InitD3D(HWND Window)
{
       // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC SwapCrainDesc;

    // clear out the struct for use
    ZeroMemory(&SwapCrainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    SwapCrainDesc.BufferCount = 1;                                    // one back buffer
    SwapCrainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    SwapCrainDesc.BufferDesc.Width = SCREEN_WIDTH;
    SwapCrainDesc.BufferDesc.Height = SCREEN_HEIGHT; 
    SwapCrainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    SwapCrainDesc.OutputWindow = Window;                                // the window to be used
    SwapCrainDesc.SampleDesc.Count = 4;                               // how many multisamples
    SwapCrainDesc.Windowed = TRUE;                                    // windowed/full-screen mode
    SwapCrainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    // create a device, device context and swap chain using the information in the scd struct
    D3D11CreateDeviceAndSwapChain(NULL,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  D3D11_SDK_VERSION,
                                  &SwapCrainDesc,
                                  &Swapchain,
                                  &DXDevice,
                                  NULL,
                                  &DXDeviceContext);

    // NOTE(barret): create the depth buffer texture
    D3D11_TEXTURE2D_DESC TexDesc;
    ZeroMemory(&TexDesc, sizeof(TexDesc));
    
    TexDesc.Width = SCREEN_WIDTH;
    TexDesc.Height = SCREEN_HEIGHT;
    TexDesc.ArraySize = 1;
    TexDesc.MipLevels = 1;
    TexDesc.SampleDesc.Count = 4;
    TexDesc.Format = DXGI_FORMAT_D32_FLOAT;
    TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D *DepthBuffer;
    DXDevice->CreateTexture2D(&TexDesc, 0, &DepthBuffer);

    // NOTE(barret): create the depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
    ZeroMemory(&DepthStencilViewDesc, sizeof(DepthStencilViewDesc));
    
    DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    DXDevice->CreateDepthStencilView(DepthBuffer, &DepthStencilViewDesc, &ZBuffer);
    DepthBuffer->Release();

    // get the address of the back buffer
    ID3D11Texture2D *TexBackBuffer;
    Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&TexBackBuffer);

    // use the back buffer address to create the render target
    DXDevice->CreateRenderTargetView(TexBackBuffer, NULL, &BackBuffer);
    TexBackBuffer->Release();

    // set the render target as the back buffer
    DXDeviceContext->OMSetRenderTargets(1, &BackBuffer, ZBuffer);

    // Set the viewport
    D3D11_VIEWPORT Viewport;
    ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
    
    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = SCREEN_WIDTH;
    Viewport.Height = SCREEN_HEIGHT;
    Viewport.MinDepth = 0;
    Viewport.MaxDepth = 1;

    DXDeviceContext->RSSetViewports(1, &Viewport);

    InitPipeline();
    InitGraphics();
    InitStates();
}

// this is the function used to render a single frame
void RenderFrame(float dt)
{
     cbuffer CBuffer;

     CBuffer.LightVector = {1.0f, 1.0f, 1.0f, 0.0f};
     CBuffer.LightColor = {0.5f, 0.5f, 0.5f, 1.0f};
     CBuffer.AmbientColor = {0.2f, 0.2f, 0.2f, 1.0f};

    D3DXMATRIX MatRotate, MatView, MatProjection;
    D3DXMATRIX MatFinal;

    static float Time = 0.0f; Time += dt;

    // create a world matrices
    D3DXMatrixRotationY(&MatRotate, Time);
 
    // create a view matrix  
    D3DXMatrixLookAtLH(&MatView,
                       &D3DXVECTOR3(0.0f, 3.0f, 5.0f),    // the camera position
                       &D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
                       &D3DXVECTOR3(0.0f, 1.0f, 0.0f));   // the up direction

    // create a projection matrix
    D3DXMatrixPerspectiveFovLH(&MatProjection,
                               (float)D3DXToRadian(45),
                               (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
                               1.0f,
                               100.0f);

    // load the matrices into the constant buffer 
    CBuffer.Final =  MatRotate * MatView * MatProjection;
    CBuffer.Rotation = MatRotate;

    // set up states
    DXDeviceContext->RSSetState(RasterState);    
    DXDeviceContext->PSSetSamplers(0, 1, &SamplerStates);
    DXDeviceContext->OMSetBlendState(BlendState, 0, 0xffffffff);

    // clear the back buffer to a deep blue
    DXDeviceContext->ClearRenderTargetView(BackBuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

    DXDeviceContext->ClearDepthStencilView(ZBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    // select which vertex buffer to display
    UINT stride = sizeof(vertex);
    UINT offset = 0;
    DXDeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXDeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // select which primtive type we are using
    DXDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw the vertex buffer to the back buffer
    DXDeviceContext->UpdateSubresource(ConstantBuffer, 0, 0, &CBuffer, 0, 0);
    DXDeviceContext->PSSetShaderResources(0, 1, &Texture);
    DXDeviceContext->DrawIndexed(36, 0, 0);

    // switch the back buffer and the front buffer
    Swapchain->Present(0, 0);
}

void CleanD3D()
{
    Swapchain->SetFullscreenState(false, 0);

    ZBuffer->Release();
    InputLayout->Release();
    VertexShader->Release();
    PixelShader->Release();
    VertexBuffer->Release();
    IndexBuffer->Release();
    ConstantBuffer->Release();
    Texture->Release();
    Swapchain->Release();
    BackBuffer->Release();
    DXDevice->Release();
    DXDeviceContext->Release();
}

// this is the function that creates the shape to render
void InitGraphics()
{
    
    vertex OurVertices[] =
        {
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},    // side 1
            {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},

            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},    // side 2
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},    // side 3
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},

            {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},    // side 4
            {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},

            {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},    // side 5
            {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

            {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},    // side 6
            {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
             
        };
    
  
    // create the vertex buffer
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));

    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    BufferDesc.ByteWidth = sizeof(vertex) * 24;             // size is the VERTEX struct * 3
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    DXDevice->CreateBuffer(&BufferDesc, NULL, &VertexBuffer);       // create the buffer

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE MappedSubResource;
    DXDeviceContext->Map(VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubResource);    // map the buffer
    memcpy(MappedSubResource.pData, OurVertices, sizeof(OurVertices));                 // copy the data
    DXDeviceContext->Unmap(VertexBuffer, NULL);                                      // unmap the buffer

    DWORD OurIndices[] =
        {
            0, 1, 2,    // side 1
            2, 1, 3,
            4, 5, 6,    // side 2
            6, 5, 7,
            8, 9, 10,    // side 3
            10, 9, 11,
            12, 13, 14,    // side 4
            14, 13, 15,
            16, 17, 18,    // side 5
            18, 17, 19,
            20, 21, 22,    // side 6
            22, 21, 23,             
        };

    // create the index buffer
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    BufferDesc.ByteWidth = sizeof(DWORD) * 36;             // size is the VERTEX struct * 3
    BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
    BufferDesc.MiscFlags = 0;
    
    DXDevice->CreateBuffer(&BufferDesc, NULL, &IndexBuffer);       // create the buffer

    DXDeviceContext->Map(IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubResource);    // map the buffer
    memcpy(MappedSubResource.pData, OurIndices, sizeof(OurIndices));                 // copy the data
    DXDeviceContext->Unmap(IndexBuffer, NULL);                                      // unmap the buffer

    D3DX11CreateShaderResourceViewFromFile(DXDevice,            // the Direct3D device
                                           "Bricks.png",    // load Wood.png in the local folder
                                           NULL,           // no additional information
                                           NULL,           // no multithreading
                                           &Texture,      // address of the shader-resource-view
                                           NULL);          // no multithreading
}


// this function loads and prepares the shaders
void InitPipeline()
{
    // load and compile the two shaders
    ID3D10Blob *VS, *PS;
    D3DX11CompileFromFile("shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

    DXDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &VertexShader);
    DXDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &PixelShader);

    DXDeviceContext->VSSetShader(VertexShader, 0, 0);
    DXDeviceContext->PSSetShader(PixelShader, 0, 0);

     // create the input layout object
    D3D11_INPUT_ELEMENT_DESC IED[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    DXDevice->CreateInputLayout(IED, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &InputLayout);
    DXDeviceContext->IASetInputLayout(InputLayout);

    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));

    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.ByteWidth = 176;
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    DXDevice->CreateBuffer(&BufferDesc, NULL, &ConstantBuffer);
    
    DXDeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
}

// initializes the states
void InitStates()
{
    D3D11_RASTERIZER_DESC rd;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;
    rd.MultisampleEnable = FALSE;
    rd.DepthBias = 0;
    rd.DepthBiasClamp = 0.0f;
    rd.SlopeScaledDepthBias = 0.0f;

    DXDevice->CreateRasterizerState(&rd, &RasterState);

    D3D11_SAMPLER_DESC sd;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MaxAnisotropy = 16;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.BorderColor[0] = 0.0f;
    sd.BorderColor[1] = 0.0f;
    sd.BorderColor[2] = 0.0f;
    sd.BorderColor[3] = 0.0f;
    sd.MinLOD = 0.0f;
    sd.MaxLOD = FLT_MAX;
    sd.MipLODBias = 0.0f;

    DXDevice->CreateSamplerState(&sd, &SamplerStates);    // create the default sampler

    D3D11_BLEND_DESC bd;
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    bd.IndependentBlendEnable = FALSE;
    bd.AlphaToCoverageEnable = FALSE;

    DXDevice->CreateBlendState(&bd, &BlendState);
}
