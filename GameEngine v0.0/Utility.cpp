#include "Utility.h"

void Utility::LoadPNGTexture()
{

}

ID3DBlob* Utility::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target)
{
	ID3DBlob *shaderByteCode;
	D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(),
		target.c_str(), D3DCOMPILE_DEBUG, 0, &shaderByteCode, nullptr);
	return shaderByteCode;
}

Font Utility::LoadFont(LPCWSTR filename, int windowWidth, int windowHeight)
{
	std::wifstream fs;
	fs.open(filename);
	Font font;
	std::wstring tmp;
	int startpos;

	fs >> tmp >> tmp;
	startpos = tmp.find(L"\"") + 1;
	font.name = tmp.substr(startpos, tmp.size() - startpos - 1);

	fs >> tmp;
	startpos = tmp.find(L"=") + 1;
	font.size = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	fs >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp;

	fs >> tmp;

	startpos = tmp.find(L"=") + 1;
	tmp = tmp.substr(startpos, tmp.size() - startpos);

	// get up padding
	startpos = tmp.find(L",") + 1;
	font.toppadding = std::stoi(tmp.substr(0, startpos));

	// get right padding
	tmp = tmp.substr(startpos, tmp.size() - startpos);
	startpos = tmp.find(L",") + 1;
	font.rightpadding = std::stoi(tmp.substr(0, startpos));

	// get down padding
	tmp = tmp.substr(startpos, tmp.size() - startpos);
	startpos = tmp.find(L",") + 1;
	font.bottompadding = std::stoi(tmp.substr(0, startpos));

	// get left padding
	tmp = tmp.substr(startpos, tmp.size() - startpos);
	font.leftpadding = std::stoi(tmp);

	fs >> tmp; // spacing=0,0

	// get lineheight (how much to move down for each line), and normalize (between 0.0 and 1.0 based on size of font)
	fs >> tmp >> tmp; // common lineHeight=95
	startpos = tmp.find(L"=") + 1;
	font.lineHeight = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	// get base height (height of all characters), and normalize (between 0.0 and 1.0 based on size of font)
	fs >> tmp; // base=68
	startpos = tmp.find(L"=") + 1;
	font.baseHeight = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	// get texture width
	fs >> tmp; // scaleW=512
	startpos = tmp.find(L"=") + 1;
	font.textureWidth = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	// get texture height
	fs >> tmp; // scaleH=512
	startpos = tmp.find(L"=") + 1;
	font.textureHeight = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	// get pages, packed, page id
	fs >> tmp >> tmp; // pages=1 packed=0
	fs >> tmp >> tmp; // page id=0

	// get texture filename
	std::wstring wtmp;
	fs >> wtmp; // file="Arial.png"
	startpos = wtmp.find(L"\"") + 1;
	font.fontImage = wtmp.substr(startpos, wtmp.size() - startpos - 1);

	// get number of characters
	fs >> tmp >> tmp; // chars count=97
	startpos = tmp.find(L"=") + 1;
	font.numCharacters = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	// initialize the character list
	font.CharList = new FontChar[font.numCharacters];

	for (int c = 0; c < font.numCharacters; ++c)
	{
		// get unicode id
		fs >> tmp >> tmp; // char id=0
		startpos = tmp.find(L"=") + 1;
		font.CharList[c].id = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get x
		fs >> tmp; // x=392
		startpos = tmp.find(L"=") + 1;
		font.CharList[c].u = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)font.textureWidth;

		// get y
		fs >> tmp; // y=340
		startpos = tmp.find(L"=") + 1;
		font.CharList[c].v = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)font.textureHeight;

		// get width
		fs >> tmp; // width=47
		startpos = tmp.find(L"=") + 1;
		tmp = tmp.substr(startpos, tmp.size() - startpos);
		font.CharList[c].width = (float)std::stoi(tmp);
		font.CharList[c].twidth = (float)std::stoi(tmp) / (float)font.textureWidth;

		// get height
		fs >> tmp; // height=57
		startpos = tmp.find(L"=") + 1;
		tmp = tmp.substr(startpos, tmp.size() - startpos);
		font.CharList[c].height = (float)std::stoi(tmp);
		font.CharList[c].theight = (float)std::stoi(tmp) / (float)font.textureHeight;

		// get xoffset
		fs >> tmp; // xoffset=-6
		startpos = tmp.find(L"=") + 1;
		font.CharList[c].xoffset = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get yoffset
		fs >> tmp; // yoffset=16
		startpos = tmp.find(L"=") + 1;
		font.CharList[c].yoffset = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get xadvance
		fs >> tmp; // xadvance=65
		startpos = tmp.find(L"=") + 1;
		font.CharList[c].xadvance = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get page
		// get channel
		fs >> tmp >> tmp; // page=0    chnl=0
	}

	// get number of kernings
	fs >> tmp >> tmp; // kernings count=96
	startpos = tmp.find(L"=") + 1;
	font.numKernings = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

	// initialize the kernings list
	font.KerningList = new FontKerning[font.numKernings];

	for (int k = 0; k < font.numKernings; ++k)
	{
		// get first character
		fs >> tmp >> tmp; // kerning first=87
		startpos = tmp.find(L"=") + 1;
		font.KerningList[k].firstid = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get second character
		fs >> tmp; // second=45
		startpos = tmp.find(L"=") + 1;
		font.KerningList[k].secondid = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get amount
		fs >> tmp; // amount=-1
		startpos = tmp.find(L"=") + 1;
		int t = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));
		font.KerningList[k].amount = (float)t;
	}

	return font;
}

XMFLOAT4X4 Utility::aiMtoxmM(aiMatrix4x4 m)
{
	return XMFLOAT4X4(m.a1, m.a2, m.a3, m.a4,
					  m.b1, m.b2, m.b3, m.b4,
					  m.c1, m.c2, m.c3, m.c4,
					  m.d1, m.d2, m.d3, m.d4);
}

XMFLOAT3 Utility::aiVtoxmV(aiVector3D v)
{
	return XMFLOAT3(v.x,v.y,v.z);
}
XMFLOAT4 Utility::aiVtoxmV(aiQuaternion v)
{
	return XMFLOAT4(v.x, v.y, v.z, v.w);
}
