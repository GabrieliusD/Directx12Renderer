Texture2D t1 : register(t0);
SamplerState s1: register(s0);

struct Material
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float roughness;
};
struct Light
{
    float3 Strenght;
    float3 Direction;
};
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));
    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPrecent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);
    return reflectPrecent;
}

float3 BlinnPhon(float3 lightStrenght, float3 lightVec, float3 normal, float3 toEye, Material mat )
{
    const float m = mat.roughness * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);
    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);
    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrenght;

}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    float3 lightVec = -L.Direction;
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrenght = L.Strenght * ndotl;
    return BlinnPhon(lightStrenght, lightVec, normal, toEye, mat);
}

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


struct VS_OUTPUT
{
	float4 posH: SV_POSITION;
    float3 posW : POSITION;
	float4 color : COLOR;
    float3 normal : NORMAL;
	float2 texCoord: TEXCOORD;
};


float4 main(VS_OUTPUT output) : SV_TARGET
{
    output.normal = normalize(output.normal);
    float4 te = t1.Sample(s1, output.texCoord);
    Material mat = {te, float3(0.1f, 0.1f, 0.1f), .5f };
    Light light = { float3(.8f, .8f, .8f), float3(0.0f, -1.0f, 0.00f) };
    float3 toEyeW = (gCpos - output.posW);
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 1.0f) * te;
    float3 directLight = ComputeDirectionalLight(light, mat, output.normal, toEyeW);
    float4 litColor = ambient + float4(directLight, 0.0f);
	return litColor;
}