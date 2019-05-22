// Tri table resource
// Creates a 2D texture to use as the triangulation table based on Paul Bourke's reference implementation
#ifndef _TRI_TABLE_RESOURCE_H_
#define _TRI_TABLE_RESOURCE_H_

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;

class TriTableTexture
{

public:

	TriTableTexture(ID3D11Device* device);
	~TriTableTexture();

	ID3D11ShaderResourceView* getTriTable();

private:

	// Function for creating the triangle table as a texture resource to be input into the geometry shader
	void createTriTableResource(ID3D11Device* device);

	ID3D11Texture2D* texture;

	// Triangle table texture resource view to be passed to the shader
	ID3D11ShaderResourceView* triTableSRV;

};

#endif // !_TRI_TABLE_RESOURCE_H_