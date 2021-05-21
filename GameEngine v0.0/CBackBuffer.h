#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <Windows.h>
#include <vector>

 

class CBackBuffer
{
private:
	int mWidth;
	int mHeight;
	int mBufferCount;
	int mCurrentBuffer = 0;
	IDXGISwapChain *mSwapChain;
	std::vector<ID3D12Resource*> mSwapChainBuffer{mBufferCount};
	HWND mMain;
	DXGI_SWAP_CHAIN_DESC sd;
	UINT mRtvDescSize;
	ID3D12Device* mDevice;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
public:
	CBackBuffer(HWND mainWin, int width, int height, int bufferCount);
	~CBackBuffer();


	DXGI_SWAP_CHAIN_DESC& getDesc() { return sd; }
	IDXGISwapChain *getSwapChain() { return mSwapChain; }
	bool CreateBufferDesc();
	bool CreateBuffer(IDXGIFactory* factory, ID3D12CommandQueue *queue);
	bool CreateRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUhandle);
	bool RecreateBuffer(int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUhandle);

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUhandle)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvCPUhandle, mCurrentBuffer, mRtvDescSize);
	}

	ID3D12Resource* CurrentBackBuffer()
	{
		return mSwapChainBuffer[mCurrentBuffer];
	}

	void SwapBuffers()
	{
		

		mSwapChain->Present(0, 0);

		mCurrentBuffer = (mCurrentBuffer + 1) % mBufferCount;
	}
};

