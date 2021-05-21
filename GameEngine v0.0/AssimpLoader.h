#pragma once

#include "Utility.h"
//#include<assimp\Importer.hpp>
#include <vector>

struct VerBoneData
{
	uint16_t IDS[4];
	float weights[4];
	
	void AddBoneData(int boneID, float Weight)
	{
		for (UINT i = 0; i < 4; i++)
		{
			if (weights[i] == 0.0)
			{
				IDS[i] = boneID;
				weights[i] = Weight;
				return;
			}
		}
		assert(0);
	}
};
class AssimpLoader
{
	//Assimp::Importer assimpimporter;

	std::vector<VerBoneData> bones;

	std::vector<BoneInfo> mBoneInfos;
	std::map<std::string, int> mBoneMapping;
	int mNumBones = 0;
	std::vector<aiMatrix4x4> transforms;
	ID3D12Device* mDevice = nullptr;
	ID3D12CommandAllocator* mAllocator = nullptr;
	ID3D12GraphicsCommandList* mCommandList = nullptr;
	ID3D12CommandQueue* mCommandQueue = nullptr;
	void interpolateHerarchy(aiNode* node);
	void LoadBones(aiMesh* mesh);
	void GetAnimations(aiScene anim);
public:
	void Start();
	void End();
	Mesh* LoadFile(std::string name);
	ID3D12Resource* UploadToBuffer(const void* initData, UINT64 byteSize, ID3D12Resource*& uploadBuffer);
};

