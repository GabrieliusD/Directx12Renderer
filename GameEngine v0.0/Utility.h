#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include "MessageLog.h"
#include <cassert>
#include "FontLoader.h"
#include <fstream>
#include <unordered_map>
#include <map>
#include "GameTimer.h"
#include "assimp/scene.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"

static int nodeCount = 0;

class Utility
{
public:
	static void LoadPNGTexture();
	static ID3DBlob* CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target);
	static Font LoadFont(LPCWSTR filename, int windowWidth, int windowHeight);
	static XMFLOAT4X4 aiMtoxmM(aiMatrix4x4 m);
	static XMFLOAT3 aiVtoxmV(aiVector3D v);
	static XMFLOAT4 aiVtoxmV(aiQuaternion q);

};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    if(FAILED(hr__)) { std::cout << "error";  } \
}
#endif

#ifndef PrintError
#define PrintError(x)\
{\
	HRESULT hr = (x); \
	if (FAILED(hr)) { MessageLog::PrintMessage(L"test",hr);} \
}
#endif

static DirectX::XMFLOAT4X4 identity4x4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

static DirectX::XMFLOAT4X4 matrix123(
	1.0f, 2.0f, 3.0f, 4.0f,
	5.0f, 6.0f, 7.0f, 8.0f,
	9.0f, 10.0f, 11.0f, 12.0f,
	13.0f, 14.0f, 15.0f, 16.0f);
struct BoneInfo
{
	XMFLOAT4X4 boneOffset = identity4x4;
	XMFLOAT4X4 FinalTransform = identity4x4;
	XMFLOAT4X4 global = identity4x4;
};

enum DrawType {
	UNSPECIFIED,
	INDEXED,
	VERTEX
};
struct SubMesh
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
	XMFLOAT4X4 Transform;
};

struct Mesh
{
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mGlobalInverseTransform;
	DrawType drawType = UNSPECIFIED;

	ID3D12Resource* mVerBuff = nullptr;
	ID3D12Resource* mIndBuff = nullptr;;

	ID3D12Resource* mVerUploadBuff = nullptr;
	ID3D12Resource* mIndUploadBuff = nullptr;
	UINT vByteSize;
	UINT vStrideSize;

	DXGI_FORMAT iFormat;
	UINT iSizeBytes;
	UINT total;
	std::vector<SubMesh> DrawArgs;
	D3D12_VERTEX_BUFFER_VIEW GetVertBuffView()
	{
		D3D12_VERTEX_BUFFER_VIEW view;
		view.BufferLocation = mVerBuff->GetGPUVirtualAddress();
		view.StrideInBytes = vStrideSize;
		view.SizeInBytes = vByteSize;

		return view;
	}

	D3D12_INDEX_BUFFER_VIEW GetIndBuffView()
	{
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = mIndBuff->GetGPUVirtualAddress();
		view.Format = iFormat;
		view.SizeInBytes = iSizeBytes;

		return view;
	}
	std::map<std::string, aiNodeAnim*> animNodes;
	std::vector<BoneInfo> mBoneInfos;
	std::map<std::string, int> mBoneMapping;
	int mNumBones = 0;
	const aiScene *scene;

	void BoneTransforms(GameTimer& gt, std::vector<XMFLOAT4X4>& Transforms)
	{
		float runningTime = (float)((double)(double)gt.TotalTime() - gt.DeltaTime());
		XMFLOAT4X4 identity = identity4x4;
		float TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond != 0 ?
			scene->mAnimations[0]->mTicksPerSecond : 25.0f;
		float TimeInTicks = runningTime * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, scene->mAnimations[0]->mDuration);
		//std::cout << AnimationTime << std::endl;
		
		ReadNodeHeirachy(AnimationTime, scene->mRootNode, identity);
		Transforms.resize(mNumBones);
		
		for (UINT i = 0; i < mNumBones; i++)
		{
			Transforms[i] = mBoneInfos[i].FinalTransform;
		}
	}

	void ReadNodeHeirachy(float animationTime, const aiNode* pNode, const XMFLOAT4X4& parentTransform)
	{
		
		std::string nodeName(pNode->mName.data);

		


			//std::cout << nodeName << std::endl;
			const aiAnimation* pAnimation = scene->mAnimations[0];
			aiMatrix4x4 trans = pNode->mTransformation;
			//aiTransposeMatrix4(&trans);
			XMFLOAT4X4 NT = Utility::aiMtoxmM(trans);

			XMMATRIX nodeTransform =
				XMLoadFloat4x4(&NT);
			//std::cout << "nodeTransform" << std::endl;
			//MessageLog::PrintMatrix(NT);
			const aiNodeAnim* pNodeAnim = animNodes[nodeName];
			if (pNodeAnim)
			{
				//animationTime = 15;
				XMFLOAT3 Scaling;
				CalcInterpolatedScaling(Scaling, animationTime, pNodeAnim);
				XMMATRIX ScalingMatrix = XMMatrixScaling(Scaling.x, Scaling.y, Scaling.z);

				aiQuaternion RotationQuat;
				CalcInterpolatedRotation(RotationQuat, animationTime, pNodeAnim);
				XMMATRIX RotationMatrix = XMMatrixRotationQuaternion(XMVectorSet(RotationQuat.x, RotationQuat.y, RotationQuat.z, RotationQuat.w));

				XMFLOAT3 Position;
				CalcInterpolatedPosition(Position, animationTime, pNodeAnim);
				XMMATRIX PositionMatrix = XMMatrixTranslation(Position.x, Position.y, Position.z);
				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

				nodeTransform = ScalingMatrix * RotationMatrix * PositionMatrix;
				nodeTransform = XMMatrixTranspose(nodeTransform);

			}
			//std::cout << "IfAnimatednodeTransform" << std::endl;
			//MessageLog::PrintMatrix(nodeTransform);

			//std::cout << "ParentTransform" << std::endl;
			//MessageLog::PrintMatrix(parentTransform);

			XMMATRIX globalTransf = XMLoadFloat4x4(&parentTransform) * nodeTransform;

			//std::cout << "GlobalTransform" << std::endl;
			//MessageLog::PrintMatrix(globalTransf);
			for (auto& p : mBoneMapping)
			{
				if (p.first == nodeName)
				{
					int BoneIndex = p.second;
					XMMATRIX boneoffset = XMLoadFloat4x4(&mBoneInfos[BoneIndex].boneOffset);
					XMMATRIX globalInverseTransf = XMLoadFloat4x4(&mGlobalInverseTransform);
					XMMATRIX finaltransf = globalInverseTransf * globalTransf * boneoffset;
					//	std::cout << "BoneOffset" << std::endl;
					//MessageLog::PrintMatrix(boneoffset);
					//std::cout << "globalInverseMatrix" << std::endl;
					//MessageLog::PrintMatrix(globalInverseTransf);
					//std::cout << "finalTransform" << std::endl;
					//MessageLog::PrintMatrix(finaltransf);
					XMStoreFloat4x4(&mBoneInfos[BoneIndex].FinalTransform, (finaltransf));

					break;
				}
			}
			//if (mBoneMapping.find(nodeName) != mBoneMapping.end())
			//{
			//	int BoneIndex = mBoneMapping[nodeName];
			//	XMMATRIX boneoffset = XMLoadFloat4x4(&mBoneInfos[BoneIndex].boneOffset);
			//	std::cout << "BoneOffset" << std::endl;
			//	MessageLog::PrintMatrix(boneoffset);
			//	XMMATRIX globalInverseTransf = XMLoadFloat4x4(&mGlobalInverseTransform);
			//	std::cout << "globalInverseMatrix" << std::endl;
			//	MessageLog::PrintMatrix(globalInverseTransf);
			//	XMMATRIX finaltransf =globalInverseTransf* globalTransf * boneoffset;
			//	std::cout << "finalTransform" << std::endl;
			//	MessageLog::PrintMatrix(finaltransf);

			//	XMStoreFloat4x4(&mBoneInfos[BoneIndex].FinalTransform, (finaltransf));
			//}
			for (UINT i = 0; i < pNode->mNumChildren; i++)
			{
				XMFLOAT4X4 GT; XMStoreFloat4x4(&GT, globalTransf);
				ReadNodeHeirachy(animationTime, pNode->mChildren[i], GT);
			}
		



	}
//	void ReadNodeHeirachy(float animationTime, const aiNode* pNode, const XMFLOAT4X4& parentTransform)
//{
//	std::string nodeName(pNode->mName.data);
//	std::cout << nodeName << std::endl;
//	const aiAnimation* pAnimation = scene->mAnimations[0];
//	aiMatrix4x4 trans = pNode->mTransformation;
//	aiTransposeMatrix4(&trans);
//	XMFLOAT4X4 NT = Utility::aiMtoxmM(trans);
//	std::cout << "nodeTransform" << std::endl;
//	MessageLog::PrintMatrix(NT);
//	XMMATRIX nodeTransform = 
//		XMLoadFloat4x4(&NT);
//
//	const aiNodeAnim* pNodeAnim = animNodes[nodeName];
//	if (pNodeAnim)
//	{
//		XMFLOAT3 Scaling;
//		CalcInterpolatedScaling(Scaling, animationTime, pNodeAnim);
//		XMMATRIX ScalingMatrix = XMMatrixScaling(Scaling.x, Scaling.y, Scaling.z);
//
//		aiQuaternion RotationQuat;
//		CalcInterpolatedRotation(RotationQuat, animationTime, pNodeAnim);
//		XMMATRIX RotationMatrix = XMMatrixRotationQuaternion(XMVectorSet(RotationQuat.x, RotationQuat.y, RotationQuat.z, RotationQuat.w));
//
//		XMFLOAT3 Position;
//		CalcInterpolatedPosition(Position, animationTime, pNodeAnim);
//		XMMATRIX PositionMatrix = XMMatrixTranslation(Position.x, Position.y, Position.z);
//		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
//
//		nodeTransform = PositionMatrix * RotationMatrix;
//		//nodeTransform = XMMatrixTranspose(nodeTransform);
//
//	}
//
//	std::cout << "ParentTransform" << std::endl;
//	MessageLog::PrintMatrix(parentTransform);
//
//
//	XMMATRIX globalTransf = XMLoadFloat4x4(&parentTransform) * nodeTransform;
//	std::cout << "GlobalTransform" << std::endl;
//	MessageLog::PrintMatrix(globalTransf);
//
//	if (mBoneMapping.find(nodeName) != mBoneMapping.end())
//	{
//		int BoneIndex = mBoneMapping[nodeName];
//		XMMATRIX boneoffset = XMLoadFloat4x4(&mBoneInfos[BoneIndex].boneOffset);
//		std::cout << "BoneOffset" << std::endl;
//		MessageLog::PrintMatrix(boneoffset);
//		XMMATRIX globalInverseTransf = XMLoadFloat4x4(&mGlobalInverseTransform);
//		std::cout << "globalInverseMatrix" << std::endl;
//		MessageLog::PrintMatrix(globalInverseTransf);
//		XMMATRIX finaltransf = globalTransf*boneoffset  ;
//		std::cout << "finalTransform" << std::endl;
//		MessageLog::PrintMatrix(finaltransf);
//
//		XMStoreFloat4x4(&mBoneInfos[BoneIndex].FinalTransform,(finaltransf));
//	}
//
//	for (UINT i = 0; i < pNode->mNumChildren; i++)
//	{
//		XMFLOAT4X4 GT; XMStoreFloat4x4(&GT, globalTransf);
//		ReadNodeHeirachy(animationTime, pNode->mChildren[i], GT);
//	}
//
//}
	//void ReadNodeHeirachy(float animationTime, const aiNode* pNode, const XMFLOAT4X4& parentTransform)
	//{
	//	
	//	std::string nodeName(pNode->mName.data);
	//	std::cout << nodeName << std::endl;
	//	const aiAnimation* pAnimation = scene->mAnimations[0];
	//	aiMatrix4x4 trans = pNode->mTransformation;
	//	//aiTransposeMatrix4(&trans);

	//	XMFLOAT4X4 NT = Utility::aiMtoxmM(trans);
	//	
	//	XMMATRIX nodeTransform = XMMatrixIdentity();
	//		//XMLoadFloat4x4(&NT);
	//		//XMMATRIX(&trans.a1);
	//	
	//	const aiNodeAnim* pNodeAnim = animNodes[nodeName];
	//	if (pNodeAnim)
	//	{
	//		XMFLOAT3 Scaling;
	//		CalcInterpolatedScaling(Scaling, animationTime, pNodeAnim);
	//		XMMATRIX ScalingMatrix = XMMatrixScaling(Scaling.x, Scaling.y, Scaling.z);

	//		aiQuaternion RotationQuat;
	//		CalcInterpolatedRotation(RotationQuat, animationTime, pNodeAnim);
	//		XMMATRIX RotationMatrix = XMMatrixRotationQuaternion(XMVectorSet(RotationQuat.x, RotationQuat.y, RotationQuat.z, RotationQuat.w));

	//		XMFLOAT3 Position;
	//		CalcInterpolatedPosition(Position, animationTime, pNodeAnim);
	//		XMMATRIX PositionMatrix = XMMatrixTranslation(Position.x, Position.y, Position.z);
	//		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	//		
	//		nodeTransform = RotationMatrix*PositionMatrix * ScalingMatrix ;
	//		//nodeTransform = XMMatrixTranspose(nodeTransform);
	//	}



	//	XMMATRIX globalTransf =  XMLoadFloat4x4(&parentTransform) * nodeTransform;

	//	if (mBoneMapping.find(nodeName) != mBoneMapping.end())
	//	{
	//		int BoneIndex = mBoneMapping[nodeName];
	//		XMMATRIX boneoffset = XMLoadFloat4x4(&mBoneInfos[BoneIndex].boneOffset);
	//		XMMATRIX finaltransf =  XMLoadFloat4x4(&mGlobalInverseTransform) *  globalTransf * boneoffset;
	//		
	//		XMStoreFloat4x4(&mBoneInfos[BoneIndex].FinalTransform,XMMatrixTranspose(finaltransf));
	//	}

	//	for (UINT i = 0; i < pNode->mNumChildren; i++)
	//	{
	//		XMFLOAT4X4 GT; XMStoreFloat4x4(&GT, globalTransf);
	//		ReadNodeHeirachy(animationTime, pNode->mChildren[i], GT);
	//	}

	//}
	UINT FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
		{
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
			{
				return i;
			}
		}
		return 0;
	}
	void CalcInterpolatedPosition(XMFLOAT3& out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumPositionKeys == 1)
		{
			out = Utility::aiVtoxmV(pNodeAnim->mPositionKeys[0].mValue);
			return;
		}

		UINT PositionIndex = FindPosition(AnimationTime, pNodeAnim);
		UINT NextPositionIndex = PositionIndex + 1;

		float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;

		const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;

		aiVector3D Delta = End - Start;
		out = Utility::aiVtoxmV(Start + Factor * Delta);
	}
	UINT FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumRotationKeys > 0);

		for (UINT i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);

		return 0;
	}

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1) {
			Out = (pNodeAnim->mRotationKeys[0].mValue);
			return;
		}

		UINT RotationIndex = FindRotation(AnimationTime, pNodeAnim);
		UINT NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
		float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
		const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
		aiQuaternion inter;
		aiQuaternion::Interpolate(inter, StartRotationQ, EndRotationQ, Factor);
		Out = inter.Normalize();
	}

	UINT FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumScalingKeys > 0);

		for (UINT i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);

		return 0;
	}

	void CalcInterpolatedScaling(XMFLOAT3& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumScalingKeys == 1) {
			Out = Utility::aiVtoxmV(pNodeAnim->mScalingKeys[0].mValue);
			return;
		}

		UINT ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
		UINT NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
		const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Utility::aiVtoxmV(Start + Factor * Delta);
	}

	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
	{
		for (UINT i = 0; i < pAnimation->mNumChannels; i++)
		{
			const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
			if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
				return pNodeAnim;
			}
		}

	return nullptr;
	}
};



struct PassConstant
{
	XMFLOAT4X4 View = identity4x4;
	XMFLOAT4X4 Proj = identity4x4;
	XMFLOAT4X4 ViewProj = identity4x4;
	XMFLOAT3 cameraPos = { 0,0,0 };
};

struct ObjectConstant
{
	XMFLOAT4X4 World = identity4x4;
};

struct SkinnedConstants
{
	XMFLOAT4X4 BoneTransforms[96];
};


struct Key
{
	bool pressed = false;
	char keyPressed = ' ';
};

struct ActiveState
{
	bool LMBPressed = false;
	WPARAM wParam;
	LPARAM lParam;
};


struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 Weight;
	DirectX::XMFLOAT4 BoneIndices;
};

//void ReadNodeHeirachy(float animationTime, const aiNode* pNode, const XMFLOAT4X4& parentTransform)
//{
//	std::string nodeName(pNode->mName.data);
//	const aiAnimation* pAnimation = scene->mAnimations[0];
//	aiMatrix4x4 trans = pNode->mTransformation;
//	aiTransposeMatrix4(&trans);
//
//	XMFLOAT4X4 NT = Utility::aiMtoxmM(trans);
//
//	XMMATRIX nodeTransform = XMLoadFloat4x4(&NT);
//
//	const aiNodeAnim* pNodeAnim = animNodes[nodeName];
//	if (pNodeAnim)
//	{
//		XMFLOAT3 Scaling;
//		CalcInterpolatedScaling(Scaling, animationTime, pNodeAnim);
//		XMMATRIX ScalingMatrix = XMMatrixScaling(Scaling.x, Scaling.y, Scaling.z);
//
//		aiQuaternion RotationQuat;
//		CalcInterpolatedRotation(RotationQuat, animationTime, pNodeAnim);
//		XMMATRIX RotationMatrix = XMMatrixRotationQuaternion(XMVectorSet(RotationQuat.x, RotationQuat.y, RotationQuat.z, RotationQuat.w));
//
//		XMFLOAT3 Position;
//		CalcInterpolatedPosition(Position, animationTime, pNodeAnim);
//		XMMATRIX PositionMatrix = XMMatrixTranslation(Position.x, Position.y, Position.z);
//		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
//
//		nodeTransform = PositionMatrix * RotationMatrix;
//	}
//
//
//
//
//	XMMATRIX globalTransf = nodeTransform * XMLoadFloat4x4(&parentTransform);
//
//	if (mBoneMapping.find(nodeName) != mBoneMapping.end())
//	{
//		int BoneIndex = mBoneMapping[nodeName];
//		XMMATRIX boneoffset = XMLoadFloat4x4(&mBoneInfos[BoneIndex].boneOffset);
//		XMMATRIX finaltransf = boneoffset * globalTransf;
//
//		XMStoreFloat4x4(&mBoneInfos[BoneIndex].FinalTransform, XMMatrixTranspose(finaltransf));
//	}
//
//	for (UINT i = 0; i < pNode->mNumChildren; i++)
//	{
//		XMFLOAT4X4 GT; XMStoreFloat4x4(&GT, globalTransf);
//		ReadNodeHeirachy(animationTime, pNode->mChildren[i], GT);
//	}
//
//}