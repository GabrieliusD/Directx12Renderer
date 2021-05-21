#pragma once
#include "Utility.h"
#include <vector>
#include "FontBuffer.h"

struct SubObject
{
	UINT WorldID = -1;
	UINT IndexCount = 0;
	UINT BaseVertexLocation = 0;
	UINT StartIndexLocation = 0;
	std::string Name;
	XMFLOAT4X4 Transform;
};

class Object
{
	
	int mObjectOffsetID = -1;
	bool mDirty = true;

	XMFLOAT4X4 mWorld = identity4x4;
	Mesh mMesh;
public:
	std::vector<SubObject> subObjects;

	void SetMesh(Mesh mesh) {
		mMesh = mesh;
		ConstructObjects();
	}
	void ConstructObjects();
	Mesh* GetMesh() { return &mMesh; }
	XMFLOAT4X4 GetWorld() { return mWorld; }
	void SetWorld(XMFLOAT4X4& world) { mWorld = world; }
	bool &isDirty() { return mDirty; }
	int GetObjectOffsetID() { return mObjectOffsetID; }
	void SetObjectOffsetID(int id) { mObjectOffsetID = id; }
	int GetSubObjectSize() { return subObjects.size(); }

};

