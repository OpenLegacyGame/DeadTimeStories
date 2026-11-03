uniform texture textureDiffuse < string UIName = "Diffuse texture"; >;

sampler2D diffuseSampler = sampler_state 
{
	texture = (textureDiffuse);
	minfilter = LINEAR; 
	magfilter = LINEAR; 
	mipfilter = LINEAR;
	addressu = WRAP;
	addressv = WRAP;
};

float4x4 mvpMatrix: WorldViewProj;

struct vertexInput 
{
	float4 pos : POSITION;
	float2 tex0 : TEXCOORD0;
};
	
struct vertexOutput 
{
	float4 pos : POSITION; 
	float2 tex0 : TEXCOORD0;
};

vertexOutput vertexShader(vertexInput input)
{
	vertexOutput output;
	output.pos = mul(input.pos, mvpMatrix);
	output.tex0 = input.tex0;
	return output;
}

float4 fragmentShader(vertexOutput input) : COLOR0
{
	return float4(tex2D(diffuseSampler, input.tex0).xyz, 1);
}

technique Default
{	
	pass p0
	{
		ZEnable = true;
		ZWriteEnable = true;
		CullMode = None;
		AlphaBlendEnable = false;
		
		VertexShader = compile vs_2_0 vertexShader();
        PixelShader = compile ps_2_0 fragmentShader();
	}
}

///////////////////////////////////////////////////////////////////////////////