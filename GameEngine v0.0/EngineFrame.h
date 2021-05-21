#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXColors.h>

#include "d3dx12.h"
#include "MessageLog.h"
#include "CBackBuffer.h"
#include "CDescriptorHeaps.h"
#include "CPsoManager.h"
#include "FontBuffer.h"
#include "ImageDecoder.h"
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#include <windowsx.h>
#include "TextBox.h"
#include "Camera.h"
#include "Object.h"
#include "AssimpLoader.h"
class EngineFrame
{
private:
	HWND mMainWin;
	CBackBuffer *mBackBuffer = nullptr;
	CDescriptorHeaps *mDescriptorHeaps= nullptr;
	CPsoManager* mPsoManager = nullptr;
	Camera camera;
	XMFLOAT2 mLastMousePos;
	IDXGIFactory4* mFactory = nullptr;
	ID3D12Device* mDevice = nullptr;

	ID3D12Fence* mFence = nullptr;
	UINT64 mCurrentFence = 0;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDesriptorSize = 0;
	UINT QualityLevel = 0;
	UINT numTextures = 0;
	//command 
	ID3D12CommandQueue* mCommandQueue = nullptr;
	ID3D12CommandAllocator* mCommandAlloc = nullptr;
	ID3D12GraphicsCommandList* mCommandList = nullptr;

	const int mBufferCount = 2;
	int mWidth;
	int mHeight;
	ID3D12Resource* mDepthStencilBuffer = nullptr;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;


	std::vector<TextBox> gui;


	ID3D12Resource* passConstant;
	BYTE* mappedPass;

	ID3D12Resource* object;
	BYTE* mappedObject;

	ID3D12Resource* boneConstant;
	BYTE* mappedBone;

	std::unordered_map<std::string, ID3D12Resource*> geoBuffers;
	D3D12_VERTEX_BUFFER_VIEW buffView;
	D3D12_INDEX_BUFFER_VIEW indexView;

	bool mUpdateCamera = true;

	std::vector<Object> objects;
	std::vector<XMFLOAT4X4> mTransforms;
public:
	EngineFrame(HWND hwnd);
	bool Initialize();
	bool CreateDevice();
	bool CreateCommandList();
	bool CreateDepthStencil();
	void OnResize();
	void FlushCommandQueue();
	void InitDrawingStuff();
	void InitCamera();
	void UpdateCamera();
	void InitializeTextureData();
	void Draw();
	void MouseMove(WPARAM wparam, int x, int y);
	void MouseInput(WPARAM wparam, int x, int y);
	void KeyTracking(Key &key);
	void InitObjectRender();
	void Update(GameTimer& gt);
	void UpdateConstantBuffs();
	void ChangeWorld();
public:
	void SetWindowSize(int width, int height)
	{
		mWidth = width;
		mHeight = height;
	}
};

