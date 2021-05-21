#include "AssimpLoader.h"


void AssimpLoader::interpolateHerarchy(aiNode* node)
{
	transforms.push_back(node->mTransformation);
	if (node->mParent != nullptr)
	{
		aiNode* pNode = node->mParent;
		interpolateHerarchy(pNode);
	}
}

void AssimpLoader::LoadBones(aiMesh* mesh)
{
	bones.resize(mesh->mNumVertices);

	for (int i = 0; i < mesh->mNumBones; i++)
	{
		int BoneIndex = 0;
		std::string BoneName(mesh->mBones[i]->mName.data);
		if (mBoneMapping.find(BoneName) == mBoneMapping.end())
		{
			BoneIndex = mNumBones;
			mNumBones++;
			BoneInfo bi;
			mBoneInfos.push_back(bi);
		}
		else
		{
			BoneIndex = mBoneMapping[BoneName];
		}
		//aiTransposeMatrix4(&mesh->mBones[i]->mOffsetMatrix);
		mBoneInfos[BoneIndex].boneOffset = Utility::aiMtoxmM((mesh->mBones[i]->mOffsetMatrix));
		mBoneMapping[BoneName] = BoneIndex;

		for (int x = 0; x < mesh->mBones[i]->mNumWeights; x++)
		{
			int VertexID = mesh->mBones[i]->mWeights[x].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[x].mWeight;

			bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}

}

void AssimpLoader::GetAnimations(aiScene anim)
{
	//anim.mAnimations[0].
}

void AssimpLoader::Start()
{
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));

	mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocator));

	mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mAllocator, nullptr, IID_PPV_ARGS(&mCommandList));

	mCommandList->Close();

	mCommandList->Reset(mAllocator, nullptr);
}

void AssimpLoader::End()
{
	HANDLE event = CreateEvent(0, 0, 0, 0);
	ID3D12Fence* fence;
	mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	mCommandList->Close();

	mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&mCommandList);

	mCommandQueue->Signal(fence, 1);

	fence->SetEventOnCompletion(1, event);
	WaitForSingleObject(event, INFINITE);
}

Mesh* AssimpLoader::LoadFile(std::string name)
{
	
	const aiScene* scene = aiImportFile(name.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);

		//assimpimporter.ReadFile(name.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights);
	Mesh *mainMesh = new Mesh;
	mainMesh->mWorld = Utility::aiMtoxmM(scene->mRootNode->mTransformation);
	aiMatrix4x4 git = scene->mRootNode->mTransformation;
	mainMesh->mGlobalInverseTransform = Utility::aiMtoxmM(git.Inverse());
	std::vector<Vertex> vertex;
	std::vector<uint16_t> indices;
	if (scene->HasMeshes())
	{
		int totalIndices = 0;
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			int indexCount = 0;
			int vertexCount = 0;
			aiMesh* mesh = scene->mMeshes[i];
			LoadBones(mesh);
			
			for (int x = 0; x < mesh->mNumVertices; x++)
			{
				
				aiVector3D pos = mesh->mVertices[x];
				aiVector3D norm = mesh->mNormals[x];
				auto uv = mesh->mTextureCoords[0][x];
				Vertex v;
				v.Pos.x = pos.x;
				v.Pos.y = pos.y;
				v.Pos.z = pos.z;
				v.Normal.x = norm.x;
				v.Normal.y = norm.y;
				v.Normal.z = norm.z;
				v.TexC.x = uv.x;
				v.TexC.y = uv.y;
				v.BoneIndices.x = bones[x].IDS[0];
				v.BoneIndices.y = bones[x].IDS[1];
				v.BoneIndices.z = bones[x].IDS[2];
				v.BoneIndices.w = bones[x].IDS[3];
				v.Weight.x = bones[x].weights[0];
				v.Weight.y = bones[x].weights[1];
				v.Weight.z = bones[x].weights[2];
				v.Weight.w = bones[x].weights[3];

				vertex.push_back(v);
				vertexCount++;
			}

			for (int y = 0; y < mesh->mNumFaces; y++)
			{
				uint16_t indice = mesh->mFaces[y].mIndices[0];
				indices.push_back(indice);
				indice = mesh->mFaces[y].mIndices[1];
				indices.push_back(indice);
				indice = mesh->mFaces[y].mIndices[2];
				indices.push_back(indice);
				indexCount += 3;
			}

			SubMesh subMesh;
			subMesh.IndexCount = indexCount;
			subMesh.BaseVertexLocation = vertex.size()-vertexCount;
			subMesh.StartIndexLocation = indices.size() - indexCount;
			//interpolateHerarchy(scene->mRootNode->FindNode(mesh->mName));
			aiMatrix4x4 finalTransform;
			//for (int i = transforms.size()-1; i >= 0; i--)
			//{
			//	aiMultiplyMatrix4(&finalTransform, &transforms[i]);
			//}
			transforms.clear();
			subMesh.Transform = Utility::aiMtoxmM(finalTransform);
			mainMesh->DrawArgs.push_back(subMesh);
			totalIndices = vertex.size();
		}
		UINT vByte = vertex.size() * sizeof(Vertex);
		UINT iByte = indices.size() * sizeof(uint16_t);
		for (int i = 0; i < scene->mAnimations[0]->mNumChannels; i++)
		{
			mainMesh->animNodes[scene->mAnimations[0]->mChannels[i]->mNodeName.data] = scene->mAnimations[0]->mChannels[i];
		}

		mainMesh->mVerBuff = UploadToBuffer(vertex.data(), vByte, mainMesh->mVerUploadBuff);
		mainMesh->mIndBuff = UploadToBuffer(indices.data(), iByte, mainMesh->mIndUploadBuff);
		mainMesh->iFormat = DXGI_FORMAT_R16_UINT;
		mainMesh->iSizeBytes = iByte;
		mainMesh->vByteSize = vByte;
		mainMesh->vStrideSize = sizeof(Vertex);
		mainMesh->total = indices.size();
		mainMesh->mBoneInfos =  mBoneInfos;
		mainMesh->mBoneMapping = mBoneMapping;
		mainMesh->mNumBones = mNumBones;
		mainMesh->scene = scene;


	}


	vertex.clear();
	indices.clear();

	return mainMesh;
}

ID3D12Resource* AssimpLoader::UploadToBuffer(const void* initData, UINT64 byteSize, ID3D12Resource*& uploadBuffer)
{
	ID3D12Resource* defaultBuffer = nullptr;
	PrintError(mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(byteSize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&defaultBuffer)));

	PrintError(mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(byteSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = byteSize;

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(mCommandList, defaultBuffer, uploadBuffer, 0, 0, 1, &subResourceData);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return defaultBuffer;
}
