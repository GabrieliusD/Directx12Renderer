#include "CPsoManager.h"

CPsoManager::CPsoManager()
{
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
}

void CPsoManager::DefinedRootParameter()
{
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 1> sampler = 
	{
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP)
	};


	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE fontTable;
	fontTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	slotRootParameter[0].InitAsDescriptorTable(1, &fontTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter,(UINT)sampler.size(), sampler.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* serializedRootSig = nullptr;
	ID3DBlob* errorBlob = nullptr;

	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);

	PrintError(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSig["font"])));


	CD3DX12_ROOT_PARAMETER rootParamObject[4];
	//pass constant
	CD3DX12_DESCRIPTOR_RANGE cbv1;
	cbv1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//obj constant
	CD3DX12_DESCRIPTOR_RANGE cbv2;
	cbv2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	//bone constants
	CD3DX12_DESCRIPTOR_RANGE cbv3;
	cbv3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	
	rootParamObject[0].InitAsDescriptorTable(1, &cbv1);
	rootParamObject[1].InitAsDescriptorTable(1, &cbv2);
	rootParamObject[2].InitAsDescriptorTable(1, &fontTable);
	rootParamObject[3].InitAsDescriptorTable(1, &cbv3);

	CD3DX12_ROOT_SIGNATURE_DESC objRootDesc(4, rootParamObject, sampler.size(), sampler.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12SerializeRootSignature(&objRootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);

	HRESULT h = mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSig["object"]));


}

void CPsoManager::CreateInputLayout()
{
	mFontLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,1},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,16,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,1},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,1}
	};

	mObjLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,48,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"BONEINDICES",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,64,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}

	};
}

void CPsoManager::CreateShaderCode()
{
	const D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1",
		NULL, NULL
	};


	mShaders["FontVS"] = Utility::CompileShader(L"Shaders/FontVS.hlsl", nullptr, "main", "vs_5_0");
	mShaders["FontPS"] = Utility::CompileShader(L"Shaders/FontPS.hlsl", nullptr, "main", "ps_5_0");

	mShaders["SkinnedVS"] = Utility::CompileShader(L"Shaders/objVS.hlsl", skinnedDefines, "main", "vs_5_0");
	mShaders["SkinnedPS"] = Utility::CompileShader(L"Shaders/objPS.hlsl", skinnedDefines, "main", "ps_5_0");

	mShaders["ObjVS"] = Utility::CompileShader(L"Shaders/objVS.hlsl", nullptr, "main", "vs_5_0");
	mShaders["ObjPS"] = Utility::CompileShader(L"Shaders/objPS.hlsl", nullptr, "main", "ps_5_0");
}

void CPsoManager::DefinedPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

	ZeroMemory(&psoDesc, sizeof(psoDesc));
	psoDesc.InputLayout = {mFontLayout.data(),(UINT)mFontLayout.size()};
	psoDesc.pRootSignature = mRootSig["font"];
	psoDesc.VS = {reinterpret_cast<BYTE*>(mShaders["FontVS"]->GetBufferPointer()), mShaders["FontVS"]->GetBufferSize()};
	psoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["FontPS"]->GetBufferPointer()), mShaders["FontPS"]->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	CD3DX12_DEPTH_STENCIL_DESC textDepthStencil(D3D12_DEFAULT);
	textDepthStencil.DepthEnable = false;

	psoDesc.DepthStencilState = textDepthStencil;

	CD3DX12_BLEND_DESC psoBlend(D3D12_DEFAULT);
	psoBlend.RenderTarget[0].BlendEnable = true;
	psoBlend.AlphaToCoverageEnable = false;
	psoBlend.IndependentBlendEnable = false;
	psoBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;

	psoBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	psoDesc.BlendState = psoBlend;
	PrintError(
	mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPso["default"])));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoObjDesc;

	ZeroMemory(&psoObjDesc, sizeof(psoObjDesc));
	psoObjDesc.InputLayout = { mObjLayout.data(),(UINT)mObjLayout.size() };
	psoObjDesc.pRootSignature = mRootSig["object"];
	psoObjDesc.VS = { reinterpret_cast<BYTE*>(mShaders["ObjVS"]->GetBufferPointer()), mShaders["ObjVS"]->GetBufferSize() };
	psoObjDesc.PS = { reinterpret_cast<BYTE*>(mShaders["ObjPS"]->GetBufferPointer()), mShaders["ObjPS"]->GetBufferSize() };
	psoObjDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoObjDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoObjDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoObjDesc.SampleMask = UINT_MAX;
	psoObjDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoObjDesc.NumRenderTargets = 1;
	psoObjDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoObjDesc.SampleDesc.Count = 1;
	psoObjDesc.SampleDesc.Quality = 0;
	psoObjDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	CD3DX12_DEPTH_STENCIL_DESC DepthStencil(D3D12_DEFAULT);

	psoObjDesc.DepthStencilState = DepthStencil;


	PrintError(
		mDevice->CreateGraphicsPipelineState(&psoObjDesc, IID_PPV_ARGS(&mPso["object"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedObjectDesc = psoObjDesc;
	skinnedObjectDesc.VS = { reinterpret_cast<BYTE*>(mShaders["SkinnedVS"]->GetBufferPointer()), mShaders["SkinnedVS"]->GetBufferSize() };
	skinnedObjectDesc.PS = { reinterpret_cast<BYTE*>(mShaders["SkinnedPS"]->GetBufferPointer()), mShaders["SkinnedPS"]->GetBufferSize() };

	mDevice->CreateGraphicsPipelineState(&skinnedObjectDesc, IID_PPV_ARGS(&mPso["skinned"]));
}
