#include "EngineFrame.h"

EngineFrame::EngineFrame(HWND hwnd) : mMainWin(hwnd)
{
	RECT rect;

	GetClientRect(mMainWin, &rect);
	
	mWidth = rect.right - rect.left;
	mHeight = rect.bottom - rect.top;
	
}

bool EngineFrame::Initialize()
{

	ID3D12Debug *spDebugController0;
	ID3D12Debug1* spDebugController1;

	D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
	spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
	spDebugController1->SetEnableGPUBasedValidation(true);
	spDebugController1->EnableDebugLayer();
	
	ID3D12DebugCommandList* COMMAND;




	//creates d3d12device
	CreateDevice();


	mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));

	//Creates command alloc, queue and list
	CreateCommandList();

	//Creates back and front buffer
	mBackBuffer = new CBackBuffer(mMainWin,mWidth,mHeight,mBufferCount);
	mBackBuffer->CreateBuffer(mFactory, mCommandQueue);

	//Clears command list
	FlushCommandQueue();

	mCommandList->Reset(mCommandAlloc, nullptr);

	//Allocates space for current descriptors needed
	mDescriptorHeaps = new CDescriptorHeaps();
	mDescriptorHeaps->CreateRTVHeapDesc(mBufferCount);
	mDescriptorHeaps->CreateDSVHeapDesc(1);
	mDescriptorHeaps->CreateCUDvVHeap(20);
	mBackBuffer->CreateRenderTargetView(mDescriptorHeaps->GetRtvCpuHandle());

	//DepthStencil
	CreateDepthStencil();
	mCbvSrvUavDesriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//Viewport and Scissor Rect
	mScreenViewport.TopLeftX = 0.0f;
	mScreenViewport.TopLeftY = 0.0f;
	mScreenViewport.Width = mWidth;
	mScreenViewport.Height = mHeight;
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0,0,mWidth,mHeight };
	

	//Send commands recorded to GPU
	mCommandList->Close();

	ID3D12CommandList* cmdLists[] = { mCommandList };

	mCommandQueue->ExecuteCommandLists(_countof(cmdLists),cmdLists);

	//Wait for Commands to Finish
	FlushCommandQueue();

	InitializeTextureData();

	return true;
}

bool EngineFrame::CreateDevice()
{
	HRESULT hResult;
	hResult = CreateDXGIFactory(IID_PPV_ARGS(&mFactory));

	MessageLog::PrintMessage(L"Initializing factory", hResult);

	IDXGIAdapter1* adapter;

	int adapterIndex = 0;
	while (mFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);

		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			continue;
		}

		hResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
		//MessageLog::PrintMessage(L"Initializing D3D Driver", hResult);
		if (SUCCEEDED(hResult))
			break;

	}
	return false;
}

bool EngineFrame::CreateCommandList()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));

	mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAlloc));

	mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAlloc, nullptr, IID_PPV_ARGS(&mCommandList));

	mCommandList->Close();

	MessageLog::PrintMessage(L"command list Created", NULL);
	
	return true;
}

bool EngineFrame::CreateDepthStencil()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear,
		IID_PPV_ARGS(&mDepthStencilBuffer));

	mDevice->CreateDepthStencilView(mDepthStencilBuffer, nullptr, mDescriptorHeaps->GetDsvCpuHandle());

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));



	return false;
}
void EngineFrame::OnResize()
{
	FlushCommandQueue();
	mCommandList->Reset(mCommandAlloc, nullptr);

	mBackBuffer->RecreateBuffer(mWidth, mHeight, mDescriptorHeaps->GetRtvCpuHandle());

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear,
		IID_PPV_ARGS(&mDepthStencilBuffer));

	mDevice->CreateDepthStencilView(mDepthStencilBuffer, nullptr, mDescriptorHeaps->GetDsvCpuHandle());

	
	
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	
	float posXper = float(mWidth) / 100.f * 50.f;
	float posYper = float(mHeight / 100.f * 50.f);
	
	gui[0].SetScreen(XMFLOAT2(mWidth, mHeight));
	gui[0].SetSize(XMFLOAT2(posXper, posYper));
	gui[0].OnResize();

	

	camera.SetLens(0.25f * 3.14f, (float)mWidth / (float)mHeight, 1.0f, 1000.0f);
	camera.UpdateViewMatrix();
	mUpdateCamera = true;
	mCommandList->Close();

	mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&mCommandList);

	FlushCommandQueue();

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = mWidth;
	mScreenViewport.Height = mHeight;
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0,0,mWidth,mHeight };
}
void EngineFrame::FlushCommandQueue()
{
	mCurrentFence++;
	mCommandQueue->Signal(mFence, mCurrentFence);
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		mFence->SetEventOnCompletion(mCurrentFence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);

		CloseHandle(eventHandle);
	}
}

void EngineFrame::InitDrawingStuff()
{
	mPsoManager = new CPsoManager();
	mPsoManager->CreateInputLayout();
	mPsoManager->CreateShaderCode();
	mPsoManager->DefinedRootParameter();
	mPsoManager->DefinedPSO();

	TextBox textBox(XMFLOAT2(mWidth, mHeight), XMFLOAT2(500,  0), XMFLOAT2(200, 70));
	AssimpLoader a;
	a.Start();
	Mesh* chair = a.LoadFile("char.fbx");
	//Mesh* chair1 = a.LoadFile("ran.fbx");
	//Mesh* box = a.LoadFile("land.fbx");
	a.End();
	gui.push_back(textBox);


	Object objchair;
	objchair.SetMesh(*chair);
	objchair.SetObjectOffsetID(0);
	objects.push_back(objchair);


	//Object newobj;
	//newobj.SetObjectOffsetID(objchair.GetObjectOffsetID() + objchair.GetSubObjectSize());
	//newobj.SetMesh(*chair1);
	//XMFLOAT4X4 world; XMStoreFloat4x4(&world, XMMatrixTranslation(0, 0, 0));
	//newobj.SetWorld(world);
	////objects.push_back(newobj);

	//Object cube;
	//cube.SetMesh(*box);
	//cube.SetObjectOffsetID(newobj.GetObjectOffsetID() + newobj.GetSubObjectSize());
	//XMStoreFloat4x4(&world, XMMatrixTranslation(0, 0, -1));
	//cube.SetWorld(world);
	//objects.push_back(cube);
}

void EngineFrame::InitCamera()
{
	camera.SetLens(0.25f * 3.14f, (float)mWidth / (float)mHeight, 500, 1000.0f);
	camera.SetPosition(0, 0, -2);
	camera.LookAt(XMFLOAT3(0, 0, -4), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 1, 0));
	
	camera.UpdateViewMatrix();
	

	 
}

void EngineFrame::UpdateCamera()
{
	float moveSpeed = 0.5f;
	if (GetAsyncKeyState('W') & 0x8000)
	{
		camera.Walk(moveSpeed);
		mUpdateCamera = true;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		camera.Walk(-moveSpeed);
		mUpdateCamera = true;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		camera.Strafe(-moveSpeed);
		mUpdateCamera = true;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		camera.Strafe(moveSpeed);
		mUpdateCamera = true;
	}
}

void EngineFrame::InitializeTextureData()
{
	numTextures = 2;
	bool test = true;
	Texture blank;
	Texture bless;
	ImageDecoder imageDec;
	imageDec.Start();
	test = imageDec.LoadImageDataFromFile(L"newarial.png", blank);
	test = imageDec.LoadImageDataFromFile(L"blessing-1x.png", bless);
	imageDec.End();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hTexture(mDescriptorHeaps->GetCUDCpuHandle());
	auto de = blank.resource->GetDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = blank.resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = blank.resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	mDevice->CreateShaderResourceView(blank.resource, &srvDesc, hTexture);

	hTexture.Offset(1, mCbvSrvUavDesriptorSize);

	auto b = bless.resource->GetDesc();
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = b.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = b.MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	
	mDevice->CreateShaderResourceView(bless.resource, &srvDesc, hTexture);

}



void EngineFrame::Draw()
{
	mCommandAlloc->Reset();
	mCommandList->Reset(mCommandAlloc, nullptr);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);
	mCommandList->ClearRenderTargetView(mBackBuffer->CurrentBackBufferView(mDescriptorHeaps->GetRtvCpuHandle()), DirectX::Colors::Azure, 0, nullptr);

	mCommandList->ClearDepthStencilView(mDescriptorHeaps->GetDsvCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
	mCommandList->OMSetRenderTargets(1, &mBackBuffer->CurrentBackBufferView(mDescriptorHeaps->GetRtvCpuHandle()), true, &mDescriptorHeaps->GetDsvCpuHandle());
	mDescriptorHeaps->SetSRVTable(mCommandList);
	mCommandList->SetGraphicsRootSignature(mPsoManager->GetRootSig("font"));
	CD3DX12_GPU_DESCRIPTOR_HANDLE gHandle(mDescriptorHeaps->GetCUDGpuHandle());
	//gHandle.Offset(1, mCbvSrvUavDesriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(0, gHandle);


	mCommandList->SetPipelineState(mPsoManager->GetPso("default"));
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	gui[0].Draw(mCommandList);

	mCommandList->SetGraphicsRootSignature(mPsoManager->GetRootSig("object"));
	gHandle.Offset(numTextures, mCbvSrvUavDesriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(0, gHandle);
	gHandle.Offset(objects.size() + 1, mCbvSrvUavDesriptorSize);

	mCommandList->SetGraphicsRootDescriptorTable(3, gHandle);

	for (int i = 0; i < objects.size(); i++)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE gTex(mDescriptorHeaps->GetCUDGpuHandle());
		gTex.Offset(1, mCbvSrvUavDesriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(2, gTex);

		CD3DX12_GPU_DESCRIPTOR_HANDLE gWorld(mDescriptorHeaps->GetCUDGpuHandle());
		auto mesh = objects[i].GetMesh();
		gWorld.Offset(numTextures, mCbvSrvUavDesriptorSize);
		gWorld.Offset(objects[i].GetObjectOffsetID(), mCbvSrvUavDesriptorSize);

		for (int x = 0; x < objects[i].subObjects.size(); x++)
		{
			gWorld.Offset(1, mCbvSrvUavDesriptorSize);
			mCommandList->SetGraphicsRootDescriptorTable(1, gWorld);
			mCommandList->SetPipelineState(mPsoManager->GetPso("object"));
			mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			mCommandList->IASetVertexBuffers(0, 1, &mesh->GetVertBuffView());
			mCommandList->IASetIndexBuffer(&mesh->GetIndBuffView());
			mCommandList->DrawIndexedInstanced(objects[i].subObjects[x].IndexCount, 1, objects[i].subObjects[x].StartIndexLocation, objects[i].subObjects[x].BaseVertexLocation, 0);

		} 

		
	}



	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffer->CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	mCommandList->Close();

	ID3D12CommandList* cmdsLists[] = { mCommandList };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	mBackBuffer->SwapBuffers();
	//mCurrentFence++;
	//mCommandQueue->Signal(mFence, mCurrentFence);
	FlushCommandQueue();
}

void EngineFrame::MouseMove(WPARAM wparam, int x, int y)
{
	if (wparam & MK_LBUTTON)
	{

		float dx = XMConvertToRadians(0.25f * (float)(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * (float)(y - mLastMousePos.y));

		camera.Pitch(dy);
		camera.RotateY(dx);

		mUpdateCamera = true;

	}

		mLastMousePos.x = x;
		mLastMousePos.y = y;
}

void EngineFrame::MouseInput(WPARAM wparam, int x, int y)
{

	mLastMousePos.x = x;
	mLastMousePos.y = y;

	gui[0].Selected(XMFLOAT2(x,y));


}

void EngineFrame::KeyTracking(Key &key)
{
	if (gui[0].GetActive())
		gui[0].addLetter(key);
}

void EngineFrame::InitObjectRender()
{
	UINT passbyte = (sizeof(PassConstant)+255) & ~255;

	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(passbyte), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&passConstant));

	passConstant->Map(0, nullptr, reinterpret_cast<void**>(&mappedPass));

	UINT objbyte = (sizeof(ObjectConstant) + 255) & ~255;
	UINT numObject = 0;
	
	for (int i = 0; i < objects.size(); i++)
	{
		numObject += objects[i].GetSubObjectSize();
	}

	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(objbyte*numObject), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&object));

	object->Map(0, nullptr, reinterpret_cast<void**>(&mappedObject));

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mDescriptorHeaps->GetCUDCpuHandle());
	handle.Offset(numTextures, mCbvSrvUavDesriptorSize);

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passConstant->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = passbyte;

	mDevice->CreateConstantBufferView(&cbvDesc, handle);

	for (int i = 0; i < numObject; i++)
	{
		cbAddress = object->GetGPUVirtualAddress();
		cbAddress += objbyte * i;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = objbyte;

		handle.Offset(1, mCbvSrvUavDesriptorSize);

		mDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
	UINT boneByte = (sizeof(SkinnedConstants) + 255) & ~255;
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(boneByte), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&boneConstant));

	boneConstant->Map(0, nullptr, reinterpret_cast<void**>(&mappedBone));

	cbAddress = boneConstant->GetGPUVirtualAddress();
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = boneByte;

	handle.Offset(1, mCbvSrvUavDesriptorSize);

	mDevice->CreateConstantBufferView(&cbvDesc, handle);
}

void EngineFrame::Update(GameTimer&gt)
{
	//MessageLog::PrintMessage(L"update", NULL);

	UpdateCamera();
	//MessageLog::PrintMessage(L"camera", NULL);
	objects[0].GetMesh()->BoneTransforms(gt, mTransforms);

	UpdateConstantBuffs();
	//MessageLog::PrintMessage(L"constant buff", NULL);
	//std::cout << nodeCount << std::endl;
	nodeCount = 0;
	//MessageLog::PrintMessage(L"bones", NULL);

}

void EngineFrame::UpdateConstantBuffs()
{
	//MessageLog::PrintMessage(L"inside const buff", NULL);

	SkinnedConstants skinnedConstants;
	std::copy(std::begin(mTransforms), std::end(mTransforms), &skinnedConstants.BoneTransforms[0]);
	memcpy(&mappedBone[0], &skinnedConstants, sizeof(skinnedConstants));

	//MessageLog::PrintMessage(L"after updatinbg bone buffer", NULL);

	for (int i = 0; i < objects.size(); i++)
	{
		for (int x = 0; x < objects[i].GetSubObjectSize(); x++)
		{
			if (objects[i].isDirty())
			{
				ObjectConstant oc;
				XMMATRIX pworld = XMLoadFloat4x4(&objects[i].GetWorld());
				XMMATRIX cworld = XMLoadFloat4x4(&objects[i].subObjects[x].Transform);
				XMStoreFloat4x4(&oc.World, XMMatrixTranspose(pworld * cworld ));
				UINT s = (sizeof(ObjectConstant) + 255) & ~255;
				UINT offset = (s * x) + (s * objects[i].GetObjectOffsetID());
				memcpy(&mappedObject[offset], &oc, sizeof(ObjectConstant));
				std::cout << offset << std::endl;
			}
		}
		objects[i].isDirty() = false;

	}	
	if (mUpdateCamera)
	{
		camera.UpdateViewMatrix();

		XMMATRIX proj = XMLoadFloat4x4(&camera.GetProj4x4f());
		XMMATRIX view = XMLoadFloat4x4(&camera.GetView4x4f());
		XMMATRIX viewProj = XMMatrixMultiply(view, proj);

		PassConstant pass;
		XMStoreFloat4x4(&pass.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&pass.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&pass.ViewProj, XMMatrixTranspose(viewProj));
		pass.cameraPos = camera.GetPosition3f();
		memcpy(&mappedPass[0], &pass, sizeof(PassConstant));

		mUpdateCamera = false;
	}
}

void EngineFrame::ChangeWorld()
{
	if (GetAsyncKeyState('U') && 0x8000 != 1)
	{
		for (int i = 0; i < objects.size(); i++)
		{

			
				XMFLOAT4X4 world = objects[0].GetWorld();

				XMMATRIX WORLD = XMLoadFloat4x4(&world);
				WORLD = XMMatrixTranslation(.1, 0, 0) * XMMatrixRotationZ(0.001) * WORLD;
				XMStoreFloat4x4(&world, WORLD);
				objects[0].SetWorld(world);
				objects[0].isDirty() = true;
			
		}

	}
}
