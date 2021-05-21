#pragma once
#pragma once
#include <DirectXMath.h>
#include <direct.h>
#include <d3d12.h>
#include <string>
using namespace DirectX;

struct TextVertex
{
	TextVertex(float r, float g, float b, float a, float u, float v, float tw, float th, float x, float y, float w, float h) : color(r, g, b, a), textCoord(u, v, tw, th), Pos(x, y, w, h) {}
	XMFLOAT4 Pos;
	XMFLOAT4 textCoord;
	XMFLOAT4 color;
};

struct FontChar
{
	int id;

	float u;
	float v;
	float twidth;
	float theight;

	float width;
	float height;

	float xoffset;
	float yoffset;
	float xadvance;
};

struct FontKerning
{
	int firstid;
	int secondid;
	float amount;
};

struct Font
{
	std::wstring name;
	std::wstring fontImage;
	int size;
	float lineHeight;
	float baseHeight;
	int textureWidth;
	int textureHeight;
	int numCharacters;
	FontChar* CharList;
	int numKernings;
	FontKerning* KerningList;
	ID3D12Resource* textureBuffer;
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;

	float leftpadding;
	float toppadding;
	float rightpadding;
	float bottompadding;

	float GetKerning(wchar_t first, wchar_t second)
	{
		for (int i = 0; i < numKernings; i++)
		{
			if ((wchar_t)KerningList[i].firstid == first && (wchar_t)KerningList[i].secondid == second)
				return KerningList[i].amount;
		}
		return 0.0f;
	}

	FontChar* GetChar(wchar_t c)
	{
		for (int i = 0; i < numCharacters; i++)
		{
			if (c == (wchar_t)CharList[i].id)
				return &CharList[i];
		}
		return nullptr;
	}
};

class FontLoader
{

};

