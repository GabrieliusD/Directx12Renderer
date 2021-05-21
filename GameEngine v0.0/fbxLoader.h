#pragma once
#include <string>
#include <vector>
#include <fbxsdk.h>
#include <DirectXMath.h>
#include <unordered_map>
#include "Utility.h"
using namespace DirectX;




class fbxLoader
{
	FbxManager* SdkManager;
	std::unordered_map<std::string, FbxScene*> mScenes;
	std::vector<std::string> mPath;
	std::unordered_map<std::string, std::vector<Vertex>> mVertex;
public:
	fbxLoader();
public:
	void importFile(std::string path);
	void Calculate(std::string fileName);
	XMFLOAT3 fbxVector3ToXMFloat3(FbxVector4 vec);
	XMFLOAT2 fbxVector2ToXMFloat2(FbxVector2 vec);

	XMFLOAT3 Normals(FbxMesh* mesh, int ControlPointIndex, int VertexCounter);
	XMFLOAT2 UVS(FbxMesh* mesh, int ControlPointIndex, int VertexCounter);
	void ConstructFaces(std::string fileName);
	std::vector<Vertex> GetVertex(std::string fileName) {
		return mVertex[fileName];
	}
};

