#include "Object.h"

void Object::ConstructObjects()
{
	for (int i = 0; i < mMesh.DrawArgs.size(); i++)
	{
		SubObject subObject;
		subObject.WorldID = i;
		subObject.BaseVertexLocation = mMesh.DrawArgs[i].BaseVertexLocation;
		subObject.IndexCount = mMesh.DrawArgs[i].IndexCount;
		subObject.StartIndexLocation = mMesh.DrawArgs[i].StartIndexLocation;
		subObject.Transform = mMesh.DrawArgs[i].Transform;

		subObjects.push_back(subObject);
	}
}
