#pragma once
#include <wincodec.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <D3Dcompiler.h>

#include <string>



struct Texture
{
	ID3D12Resource* resource;
	ID3D12Resource* uploadHeap;
};

struct ImageData
{
	ImageData(std::wstring file) : fileName(file) {}
	std::wstring fileName;
	BYTE* imageData;
	UINT ImageWidth, ImageHeight;
	DXGI_FORMAT format;
	int BytesPerRow;
	int imageSize;
};


class ImageDecoder
{
	ID3D12Device* mDevice = nullptr;
	ID3D12CommandAllocator* mAllocator = nullptr;
	ID3D12GraphicsCommandList* mCommandList = nullptr;
	ID3D12CommandQueue* mCommandQueue = nullptr;

	 DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	 WICPixelFormatGUID getConvertToWicFormat(WICPixelFormatGUID& wicFormatGUID);
	 int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
public:
	void Start();
	void End();
	bool LoadImageDataFromFile(std::wstring filePath, Texture &tex);
private:
	void UploadTexture(ImageData& imageData, Texture &tex);
};

