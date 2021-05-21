#include "TextBox.h"

TextBox::TextBox(XMFLOAT2 screen, XMFLOAT2 pos, XMFLOAT2 size) :
	mScreen(screen), mPos(pos), mSize(size)
{
	CalculateDimensions();
	mFont = Utility::LoadFont(L"newarial.fnt", screen.x, screen.y);;
	TextVertex v(0.4f,0.9f,0.4f,1.0f,0.0f,0.0f,.9f,.9f,mDim.x,mDim.y,mDim.z,mDim.w);
	FontBuffer fontBuffer;

	UINT byteSize = mBufferSize * sizeof(TextVertex);

	buffer = fontBuffer.CreateBuffer(byteSize);

	bufferView.BufferLocation = buffer->GetGPUVirtualAddress();
	bufferView.StrideInBytes = sizeof(TextVertex);
	bufferView.SizeInBytes = byteSize;


	buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped));

	memcpy(&mapped[0], &v, sizeof(TextVertex));

	mOffset = sizeof(TextVertex);
}

void TextBox::Selected(XMFLOAT2 pos)
{

	if (mPos.x <= pos.x && pos.x <= (mPos.x + mSize.x)
		&& mPos.y <= pos.y && pos.y <=(mPos.y + mSize.y))
	{
		mSelected = true;
	}
	else mSelected = false;
}

void TextBox::Recalculate()
{
	CalculateDimensions();
}

void TextBox::SetPosition(XMFLOAT2 pos)
{
	mPos = pos;
}

void TextBox::SetSize(XMFLOAT2 size)
{
	mSize = size;
}

void TextBox::SetScreen(XMFLOAT2 screen)
{
	mLastScreen = mScreen;
	mScreen = screen;
	if (mScale)
	{
		float increase = mScreen.x - mLastScreen.x;
		float percent = increase / mLastScreen.x;
		if (percent != 0)
			scale.x = scale.x * (1 + percent);
	}

}

void TextBox::SetText(std::wstring text)
{
	//mTextLenght = text.size() + 1;
	mText = text;
	XMFLOAT2 padding(0.9f, 0.9f);
	XMFLOAT4 color(0.2f, 0.2f, 0.7f, 1.0f);
	std::vector<TextVertex> textVertices;



	int numOfCharacters = 0;
	mLastScreen;
	float topLeftScreenX = ((mPos.x / mScreen.x) * 2.0f) - 1.0f;
	float topLeftScreenY = ((1.0f - (mPos.y / mScreen.y)) * 2.0f) - 1.0f;

	float x = topLeftScreenX;
	float y = topLeftScreenY;

	float perX = mScreen.x / 100.0f * 50.0f;

	float horrizontalPadding = (mFont.leftpadding / mScreen.x + mFont.rightpadding / mScreen.x) * padding.x;
	float verticalPadding = (mFont.toppadding / mScreen.y + mFont.bottompadding / mScreen.y) * padding.y;

	wchar_t lastChar = -1;

	float furthestX = -1.0f;



	for (int i = 0; i < text.size(); ++i)
	{
		wchar_t c = text[i];

		FontChar* fc = mFont.GetChar(c);

		if (c == L'\r')
		{
			furthestX = x + 2 + topLeftScreenX;
			x = topLeftScreenX;
			y -= (mFont.lineHeight/mScreen.y + verticalPadding) * scale.y;
			continue;
		}
		if (fc == nullptr)
			continue;
		if (c == L' ')
		{
			float w = 40.2f/ mScreen.x * scale.x;
			textVertices.push_back(TextVertex(color.x, color.y, color.z, color.w,
				0, 0, 0, 0,
				x, y, w, 0));
			x += w;

			numOfCharacters++;
			mTextLenght = numOfCharacters + 1;

			continue;
		}


		if (numOfCharacters >= 1024)
			break;

		float kerning = 0.0f;
		if (i > 0)
			kerning = mFont.GetKerning(lastChar, c)/ mScreen.x;

		

		textVertices.push_back(TextVertex(color.x, color.y, color.z, color.w,
			fc->u, fc->v, fc->twidth, fc->theight,
			x + (((fc->xoffset / mScreen.x) + kerning) * scale.x),
			y - (fc->yoffset / mScreen.y * scale.y),
			(fc->width / mScreen.x) * scale.x,
			fc->height / mScreen.y*scale.y));

		numOfCharacters++;

		x += (fc->xadvance/ mScreen.x - horrizontalPadding) * scale.x;
		mLastPos.x = x;
		lastChar = c;
		mTextLenght = numOfCharacters + 1;

	}
	UINT bytesize = textVertices.size() * sizeof(TextVertex);

	memcpy(&mapped[mOffset], textVertices.data(), bytesize);


}

void TextBox::addLetter(Key& key)
{
	if ((GetKeyState(VK_BACK) & 0x8000) != 0)
	{
		wchar_t l = mText.back();
		mText.pop_back();
		if(l != '\r')
		mTextLenght--;
		return;
	}


	if (((GetKeyState(VK_SHIFT) & 0x8000) != 0) || (GetKeyState(VK_SPACE) & 0x8000) != 0 || (GetKeyState(VK_RETURN) & 0x8000))
	{
		key.keyPressed = key.keyPressed - 32;
	}

	char orig = key.keyPressed;
	size_t newSize = sizeof(orig) + 1;
	wchar_t* wlet = new wchar_t[newSize];
	mbstowcs_s(0, wlet, newSize, &orig, _TRUNCATE);
	if(orig != static_cast<char>(VK_SHIFT))
	SetText(mText + wlet);
	delete[](wlet);
}

void TextBox::Draw(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &bufferView);
	commandList->DrawInstanced(4, mTextLenght, 0, 0);
}

void TextBox::OnResize()
{

	

	CalculateDimensions();
	TextVertex v(0.1f, 0.9f, 0.4f, .2f, 0.99f, 0.99f, 0.01f, 0.01f, mDim.x, mDim.y, mDim.z, mDim.w);
	float posXper = float(mScreen.x) / 100.f * 50.f;
	float posYper = float(mScreen.y / 100.f * 50.f);

	mFont = Utility::LoadFont(L"newarial.fnt", mScreen.x, mScreen.y);
	memcpy(&mapped[0], &v, sizeof(TextVertex));

	SetText(mText);
}



void TextBox::CalculateDimensions()
{
	float sppx = 2 * (mPos.x / mScreen.x) - 1;
	float sppy = (2 * (mPos.y / mScreen.y) - 1) * -1;

	float spw = 2*(mSize.x / mScreen.x);
	float sph = 2*(mSize.y / mScreen.y);

	mDim = { sppx, sppy, spw, sph };

}
