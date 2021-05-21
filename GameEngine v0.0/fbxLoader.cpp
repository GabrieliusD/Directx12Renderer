#include "fbxLoader.h"

fbxLoader::fbxLoader()
{
	SdkManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(SdkManager, IOSROOT);
	SdkManager->SetIOSettings(ios);
}

void fbxLoader::importFile(std::string path)
{
	FbxImporter* Importer = FbxImporter::Create(SdkManager, "");
	Importer->Initialize(path.c_str(), -1, SdkManager->GetIOSettings());
	FbxScene* Scene = FbxScene::Create(SdkManager, path.c_str());
	Importer->Import(Scene);

	//check if works
	mScenes[path] = std::move(Scene);
	mPath.push_back(path);
	Importer->Destroy(); 

}

void fbxLoader::Calculate(std::string fileName)
{
	FbxNode* RootNode = mScenes[fileName]->GetRootNode();
	FbxMesh* MeshNode = nullptr;
	std::vector<FbxMesh*> m;
	if (RootNode)
	{
		for (int i = 0; i < RootNode->GetChildCount(); i++)
		{
			FbxNodeAttribute::EType at = RootNode->GetChild(i)->GetNodeAttribute()->GetAttributeType();
			if (at == FbxNodeAttribute::eMesh)
			{
				MeshNode = RootNode->GetChild(i)->GetMesh();
				m.push_back(MeshNode);
			}
		}
	}

	int polyCount = MeshNode->GetPolygonCount();
	int numOfVerts = 0;
	
	for (int i = 0; i != polyCount; i++)
	{
		numOfVerts += MeshNode->GetPolygonSize(i);
	}

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> uvs;
	std::vector<Vertex> vertex;
	int* in = MeshNode->GetPolygonVertices();
	for (int i = 0; i < numOfVerts; i++)
	{
		XMFLOAT3 position;
		position.x = MeshNode->GetControlPointAt(*in).mData[0];
		position.y = MeshNode->GetControlPointAt(*in).mData[1];
		position.z = MeshNode->GetControlPointAt(*in).mData[2];

		positions.push_back(position);
		in++;

		XMFLOAT3 normal = Normals(MeshNode, 0, i);
		normals.push_back(normal);
		uvs.push_back(UVS(MeshNode, 0, i));
	}
	int currentFace = 0;
	for (int i = 0; i  != polyCount; i++)
	{
		int polySize = MeshNode->GetPolygonSize(i);

	

			if (polySize <= 10)
			{


				int loops = polySize - 4 + 1;
				int lastNum = currentFace;
				Vertex ver;

				ver.Pos = positions[currentFace];
				ver.Normal = normals[currentFace];
				ver.TexC = uvs[currentFace];
				vertex.push_back(ver);

				ver.Pos = positions[currentFace + 1];
				ver.Normal = normals[currentFace + 1];
				ver.TexC = uvs[currentFace + 1];
				vertex.push_back(ver);

				ver.Pos = positions[currentFace + 2];
				ver.Normal = normals[currentFace + 2];
				ver.TexC = uvs[currentFace + 2];
				vertex.push_back(ver);
				int secondNum = currentFace + 2;
				int thirdNum = currentFace + 3;
				while (loops)
				{

					ver.Pos = positions[lastNum];
					ver.Normal = normals[lastNum];
					ver.TexC = uvs[lastNum];
					vertex.push_back(ver);

					ver.Pos = positions[secondNum];
					ver.Normal = normals[secondNum];
					ver.TexC = uvs[secondNum];
					vertex.push_back(ver);

					ver.Pos = positions[thirdNum];
					ver.Normal = normals[thirdNum];
					ver.TexC = uvs[thirdNum];
					vertex.push_back(ver);

					secondNum++;
					thirdNum++;
					loops--;
				}

			currentFace += polySize;
			}

		}
	
	UINT siz = vertex.size() * sizeof(Vertex);
	mVertex[fileName] = vertex;


}

XMFLOAT3 fbxLoader::fbxVector3ToXMFloat3(FbxVector4 vec)
{
	XMFLOAT3 xmf3;
	xmf3.x = vec.mData[0];
	xmf3.y = vec.mData[1];
	xmf3.z = vec.mData[2];
	return xmf3;
}

XMFLOAT2 fbxLoader::fbxVector2ToXMFloat2(FbxVector2 vec)
{
	XMFLOAT2 xmf2;
	xmf2.x = vec.mData[0];
	xmf2.y = vec.mData[1];
	return xmf2;
}

XMFLOAT3 fbxLoader::Normals(FbxMesh* mesh, int ControlPointIndex, int VertexCounter)
{
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);
	XMFLOAT3 normal = {0,0,0};
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			normal = fbxVector3ToXMFloat3(vertexNormal->GetDirectArray().GetAt(ControlPointIndex));
			break;
		case FbxGeometryElement::eIndexToDirect:
			int index = vertexNormal->GetIndexArray().GetAt(ControlPointIndex);
			normal = fbxVector3ToXMFloat3(vertexNormal->GetDirectArray().GetAt(index));
			break;
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			normal = fbxVector3ToXMFloat3(vertexNormal->GetDirectArray().GetAt(VertexCounter));
			break;
		case FbxGeometryElement::eIndexToDirect:
			int index = vertexNormal->GetIndexArray().GetAt(VertexCounter);
			normal = fbxVector3ToXMFloat3(vertexNormal->GetDirectArray().GetAt(index));
			break;
		}
		break;
	}

	return normal;
}

XMFLOAT2 fbxLoader::UVS(FbxMesh* mesh, int ControlPointIndex, int VertexCounter)
{
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);
	XMFLOAT2 uv = { 0,0 };
	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			uv = fbxVector2ToXMFloat2(vertexUV->GetDirectArray().GetAt(ControlPointIndex));
			break;
		case FbxGeometryElement::eIndexToDirect:
			int index = vertexUV->GetIndexArray().GetAt(ControlPointIndex);
			uv = fbxVector2ToXMFloat2(vertexUV->GetDirectArray().GetAt(index));
			break;
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			uv = fbxVector2ToXMFloat2(vertexUV->GetDirectArray().GetAt(VertexCounter));
			break;
		case FbxGeometryElement::eIndexToDirect:
			int index = vertexUV->GetIndexArray().GetAt(VertexCounter);
			uv = fbxVector2ToXMFloat2(vertexUV->GetDirectArray().GetAt(index));
			break;
		}
		break;
	}

	
	return uv;
}

void fbxLoader::ConstructFaces(std::string fileName)
{

}
