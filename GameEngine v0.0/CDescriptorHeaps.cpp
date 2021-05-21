#include "CDescriptorHeaps.h"

CDescriptorHeaps::CDescriptorHeaps()
{
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
}

void CDescriptorHeaps::CreateRTVHeapDesc(int descCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = descCount;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));

	
}

void CDescriptorHeaps::CreateDSVHeapDesc(int descCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = descCount;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSvHeap));
}

void CDescriptorHeaps::CreateCUDvVHeap(int descCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC CUDheapdesc;
	CUDheapdesc.NumDescriptors = descCount;
	CUDheapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CUDheapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CUDheapdesc.NodeMask = 0;

	PrintError(mDevice->CreateDescriptorHeap(&CUDheapdesc, IID_PPV_ARGS(&mCbvUavSrvHeap)));
}


