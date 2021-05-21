#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <string>
#include "d3dx12.h"
#include <vector>
#include "Utility.h"
#include <array>
class CPsoManager
{
	ID3D12Device* mDevice;
	std::unordered_map<std::string, ID3D12PipelineState*> mPso;
	std::unordered_map<std::string, ID3DBlob*> mShaders;
	std::unordered_map<std::string, ID3D12RootSignature*> mRootSig;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mFontLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mObjLayout;
public:
	CPsoManager();
	void DefinedRootParameter();
	void CreateInputLayout();
	void CreateShaderCode();
	void DefinedPSO();
	ID3D12PipelineState* GetPso(const std::string name) { return mPso[name]; }
	ID3D12RootSignature* GetRootSig(const std::string name) { return mRootSig[name]; }
	
};

