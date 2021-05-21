Texture2D t1 : register(t0);
SamplerState s1: register(s0);

struct VS_INPUT
{
	float4 pos : POSITION;
	float4 texCoord: TEXCOORD;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color : COLOR;
	float2 texCoord: TEXCOORD;
};


VS_OUTPUT main(VS_INPUT input, uint vertexID : SV_VertexID)
{
	VS_OUTPUT output;
	
	float2 uv = float2(vertexID & 1, (vertexID >> 1) & 1);

	output.pos = float4(input.pos.x + (input.pos.z * uv.x), input.pos.y - (input.pos.w * uv.y), 0, 1);
	output.color = input.color;
	output.texCoord = float2(input.texCoord.x + (input.texCoord.z * uv.x), input.texCoord.y + (input.texCoord.w * uv.y));

	return output;
}