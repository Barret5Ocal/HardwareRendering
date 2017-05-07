cbuffer ConstantBuffer
{
    float4x4 Projection;
    float4 lightvec;
    float4 lightcol;
    float4 ambientcol; 
}

Texture2D Texture;
SamplerState ss;

struct VOut
{
    
    float4 color : COLOR;
    float2 texcoord : TEXCOORD; 
    float4 position : SV_POSITION;
};

VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
    VOut output;

    output.position = mul(Projection, position);

    output.color = ambientcol;     

    output.texcoord = texcoord;

    return output;
}


float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
    return color * Texture.Sample(ss, texcoord);
}
