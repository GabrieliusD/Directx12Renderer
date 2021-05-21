#include "CBackBuffer.h"



CBackBuffer::CBackBuffer(HWND mainWin, int width, int height, int bufferCount) : mMain(mainWin),
mWidth(width), mHeight(height), mBufferCount(bufferCount)
{
	D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&mDevice));

	CreateBufferDesc();
}


CBackBuffer::~CBackBuffer()
{
}

bool CBackBuffer::CreateBufferDesc()
{
	sd.BufferDesc.Width = mWidth;
	sd.BufferDesc.Height = mHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = mBufferCount;
	sd.OutputWindow = mMain;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	
	
	return false;
}

bool CBackBuffer::CreateBuffer(IDXGIFactory * factory, ID3D12CommandQueue* queue)
{
	factory->CreateSwapChain(queue, &sd, &mSwapChain);
	return false;
}

bool CBackBuffer::CreateRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUhandle)
{
	mRtvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvh(rtvCPUhandle);
	for (UINT i = 0; i < mBufferCount; i++)
	{
		
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i]));
		mDevice->CreateRenderTargetView(mSwapChainBuffer[i], nullptr, rtvh);
		rtvh.Offset(1, mRtvDescSize);
	}

	return false;
}

bool CBackBuffer::RecreateBuffer(int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUhandle)
{
	for (int i = 0; i < mBufferCount; i++)
	{
		mSwapChainBuffer[i]->Release();
	}
	mSwapChain->ResizeBuffers(mBufferCount, width, height, mFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	mCurrentBuffer = 0;

	CreateRenderTargetView(rtvCPUhandle);



	return false;
}


