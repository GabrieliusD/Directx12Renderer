#pragma once
#include <d3d12.h>
#include "Utility.h"
class CDescriptorHeaps
{
	ID3D12Device *mDevice;
	ID3D12DescriptorHeap* mRtvHeap;
	ID3D12DescriptorHeap* mDSvHeap;
	ID3D12DescriptorHeap* mCbvUavSrvHeap;

public:
	CDescriptorHeaps();
	void CreateRTVHeapDesc(int descCount);
	void CreateDSVHeapDesc(int descCount);
	void CreateCUDvVHeap(int descCount);

	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle()
	{
		return mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvCpuHandle()
	{
		return mDSvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCUDCpuHandle()
	{
		return mCbvUavSrvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetCUDGpuHandle()
	{
		return mCbvUavSrvHeap->GetGPUDescriptorHandleForHeapStart();
	}

	void SetSRVTable(ID3D12GraphicsCommandList* commandList)
	{
		commandList->SetDescriptorHeaps(1, &mCbvUavSrvHeap);
	}
};

