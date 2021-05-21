cbuffer object : register(b1)
{
	float4x4 gWorld;
};

cbuffer PassConst : register(b0)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float3 gCpos;
};

cbuffer BoneConst : register(b2)
{
    float4x4 gBoneTransf[96];
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float2 texCoord: TEXCOORD;
	float3 normal: NORMAL;
	float4 color : COLOR;
    float4 weight : WEIGHTS;
    float4 boneindice : BONEINDICES;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
    float3 posW : POSITION;
	float4 color : COLOR;
    float3 normal : NORMAL;
	float2 texCoord: TEXCOORD;
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
    float4 boneIDs = input.boneindice;
    float4 weight = input.weight;
	
    float3 posL = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i != 4; i++)
    {
        posL += weight[i] * mul(float4(input.pos, 1.0f), gBoneTransf[boneIDs[i]]).xyz;
    }
    input.pos = posL;
	
    float4 posW = mul(float4(input.pos, 1.0f), gWorld);
    output.posW = posW.xyz;

    output.pos = mul(posW, gViewProj);
    output.normal = mul(input.normal, (float3x3)gWorld);
	output.texCoord = input.texCoord;
	
	
	return output;
}