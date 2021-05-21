#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <string>
#include "d3dx12.h"
#include <vector>
#include "Utility.h"
class FontBuffer
{
	ID3D12Device* mDevice;
public:
	FontBuffer();
	ID3D12Resource *CreateStaticBuffer(ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, ID3D12Resource* uploadBuffer);
	ID3D12Resource* CreateBuffer(UINT64 size);
private:

};

