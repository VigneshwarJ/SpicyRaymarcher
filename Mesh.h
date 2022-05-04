#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Vertex.h"


class Mesh
{
public:
	//Mesh(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, Microsoft::WRL::ComPtr<ID3D12Device> device);
	Mesh(const char* objFile, Microsoft::WRL::ComPtr<ID3D12Device> device);
	~Mesh(void);

	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer() { return vb; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer() { return ib; }
	int GetIndexCount() { return numIndices; }
	int GetVertexCount() { return numIndices; }

	void SetBuffersAndDraw(Microsoft::WRL::ComPtr<ID3D12Resource> context);

	D3D12_VERTEX_BUFFER_VIEW GetVertBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vb;
	Microsoft::WRL::ComPtr<ID3D12Resource> ib;
	int numIndices;
	int numVertices;

	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;

	//void CreateBuffers(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, Microsoft::WRL::ComPtr<ID3D12Device> device);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

};

