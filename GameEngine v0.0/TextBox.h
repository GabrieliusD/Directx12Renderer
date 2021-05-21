#pragma once
#include <DirectXMath.h>
#include "Utility.h"
#include "FontLoader.h"
#include "FontBuffer.h"
#include <vector>
using namespace DirectX;
class TextBox
{
	///pixels
	XMFLOAT2 mScreen;
	XMFLOAT2 mLastScreen;
	XMFLOAT2 mPos;
	XMFLOAT2 mSize;
	XMFLOAT4 mTextColor;
	XMFLOAT4 mBackGroundColor;
	XMFLOAT4 mDim;
	bool mScale = false;
	XMFLOAT2 scale = { .2f, .2f };

	int mFontIndex = -1;
	bool mSelected = false;
	XMFLOAT2 mLastPos;

	int mTextLenght = 1;
	std::wstring mText;
	Font mFont;
	UINT mOffset = 0;
	UINT mBufferSize = 1024;
	ID3D12Resource *buffer;
	ID3D12Resource *uploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW bufferView;
	BYTE* mapped;

	void CalculateDimensions();

public:
	TextBox(XMFLOAT2 screen, XMFLOAT2 pos, XMFLOAT2 size);
	void Selected(XMFLOAT2 pos);
	void Recalculate();


	void SetPosition(XMFLOAT2 pos);
	void SetSize(XMFLOAT2 size);
	void SetScreen(XMFLOAT2 screen);

	void SetText(std::wstring text);
	void addLetter(Key& key);


	void Draw(ID3D12GraphicsCommandList *commandList);
	void OnResize();

	XMFLOAT4 GetScreenDimensions()
	{
		return mDim;
	}
	bool GetActive() { return mSelected; }
	std::wstring GetText()
	{
		return mText;
	}

};

