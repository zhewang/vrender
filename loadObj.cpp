////////////////////////////////////////////////////////////////////
//	readObjFile
////////////////////////////////////////////////////////////////////

#include <math.h>
#include "vgl.h"
// switched from vmath.h to glm
//#include "vmath.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;


typedef struct {
	char name[32];		// identifier
	float ambient[3];	// ambient color, r,g,b
	float diffuse[3];	// diffuse color, r,g,b
	float specular[3];	// specular color, r,g,b
} colorValues;

vector<colorValues> colorTable;

class three {
public:
	// constructor with three parameters
	three(float f1, float f2, float f3) {
		x = f1;
		y = f2;
		z = f3;
	}
	// constructor with no parameters 
	three() {};
	// member data
	float x;
	float y;
	float z;
};

//////////////////////////////////////////////////////////////////////////
//	loadObjFile
//////////////////////////////////////////////////////////////////////////
int loadObjFile(char *fileName, float *bounds, GLuint *VAO, GLuint *VAOsize)
{
	ifstream objFile;
	char keyword[32];
	char *token, *token2;
	char line[1024];
	int totalVertexCount = 0;
	bool computeNormals = false;
	int facesCount = 0;
	int verticesCount = 0;

	float ambientSelected[3] = { .1f, .1f, .1f };
	float diffuseSelected[3] = { 1.f, 1.f, 1.f };
	bool firstColor = true;
	colorValues tempColor;

	bounds[0] = bounds[2] = bounds[4] = FLT_MAX;
	bounds[1] = bounds[3] = bounds[5] = FLT_MIN;

	objFile.open(fileName, ios::in);
	if (!objFile.is_open())
	{
		cerr << "unable to read obj file: " << fileName << endl;
		return 0;
	}

	cout << "Reading obj file " << fileName << endl;

	// get path of objFileName
	char pathToFile[256];       // path to input obj file
	char *pch, *pchTemp, *pchTemp2;
	pchTemp = fileName;
	pchTemp2 = pathToFile;
	pch = strrchr(fileName, '/');
	if (pch != NULL)
	{
		while (pchTemp != pch)
			*(pchTemp2++) = *(pchTemp++);
		*(pchTemp2++) = '/';
		*(pchTemp2) = '\0';
	}
	else
	{
		pathToFile[0] = '\0';
	}
	
	// we have been able to open a wavefront obj file, so set up and return a VAO for this object described in the OBJ file
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
	
	three tempFloats;

	vector <three> vertices;	// these three dynamic arrays hold the index values read from the obj file
	vector <three> textureIndices;
	vector <three> normals;

	vector<float> vPosition;	// these dynamic arrays hold the values to be put into to OpenGL VBOs
	vector<float> vAmbientColor;
	vector<float> vDiffuseColor;
	vector<float> vNormal;

	while (!objFile.eof())
	{
		objFile >> keyword;

		if (strcmp(keyword, "mtllib") == 0)	// material file name
		{
			objFile >> keyword;	// name of material file
			// need to process the material file, hmmm...
			objFile.getline(line, 1024);
			char fullPath[128];
			strcpy(fullPath, pathToFile);
			strcat(fullPath, keyword);

			ifstream mtlFile;
			mtlFile.open(fullPath, ios::in);
			if (!mtlFile.is_open())
			{
				cerr << "could not read mtllib " << fullPath << endl;
				cerr << "default color is white " << endl;
			}
			else
			{
				while (!mtlFile.eof())
				{
					mtlFile >> keyword;
					if (strcmp(keyword, "newmtl") == 0)
					{
						if (firstColor)
							firstColor = false;
						else
							colorTable.push_back(tempColor);

						mtlFile >> tempColor.name;
					}
					else if (strcmp(keyword, "Ka") == 0)
					{
						mtlFile >> tempColor.ambient[0];
						mtlFile >> tempColor.ambient[1];
						mtlFile >> tempColor.ambient[2];
					}
					else if (strcmp(keyword, "Kd") == 0)
					{
						mtlFile >> tempColor.diffuse[0];
						mtlFile >> tempColor.diffuse[1];
						mtlFile >> tempColor.diffuse[2];
					}
					else if (strcmp(keyword, "Ks") == 0)
					{
						mtlFile >> tempColor.specular[0];
						mtlFile >> tempColor.specular[1];
						mtlFile >> tempColor.specular[2];
					}
					else // unrecognized token, so just skip the line
					{
						mtlFile.getline(line, 1024);
					}
				}
				colorTable.push_back(tempColor);
			}
		}
		else if (strcmp(keyword, "usemtl") == 0)
		{
			objFile >> keyword;		// name of selected color table entry
			// search table to find entry
			for (unsigned int i = 0; i < colorTable.size(); i++)
			{
				if (strcmp(keyword, colorTable[i].name) == 0)
				{
					ambientSelected[0] = colorTable[i].ambient[0];
					ambientSelected[1] = colorTable[i].ambient[1];
					ambientSelected[2] = colorTable[i].ambient[2];
					diffuseSelected[0] = colorTable[i].diffuse[0];
					diffuseSelected[1] = colorTable[i].diffuse[1];
					diffuseSelected[2] = colorTable[i].diffuse[2];
					break;	// stop with the first one found
				}
			}
		}
		else if (strcmp(keyword, "v") == 0)	// we have a vertex
		{
			// assume three more values, the x,y,z coordinates
			objFile >> tempFloats.x >> tempFloats.y >> tempFloats.z;
			objFile.getline(line, 128);
			vertices.push_back(tempFloats);
			verticesCount++;
			bounds[0] = tempFloats.x < bounds[0] ? tempFloats.x : bounds[0];
			bounds[1] = tempFloats.x > bounds[1] ? tempFloats.x : bounds[1];
			bounds[2] = tempFloats.y < bounds[2] ? tempFloats.y : bounds[2];
			bounds[3] = tempFloats.y > bounds[3] ? tempFloats.y : bounds[3];
			bounds[4] = tempFloats.z < bounds[4] ? tempFloats.z : bounds[4];
			bounds[5] = tempFloats.z > bounds[5] ? tempFloats.z : bounds[5];
		}
		else if (strcmp(keyword, "vt") == 0)	// texture map indices
		{
			objFile >> tempFloats.x >> tempFloats.y;
			tempFloats.z = -1.0f;
			textureIndices.push_back(tempFloats);
		}
		else if (strcmp(keyword, "vn") == 0)	// vertex normal indices
		{
			objFile >> tempFloats.x >> tempFloats.y >> tempFloats.z;
			normals.push_back(tempFloats);
		}
		else if (strcmp(keyword, "f") == 0)	// a face or triangle description
		{
			// f  and first reference is vertex id, second is texture id, and third is normal id
			// second and third ids are optional   
			// it is possible to have two // together, such as 3//1
			objFile.getline(line, 1024);
			// break out the information of indices for vertices, textures and normals
			token = strtok(line, " ");

			// maintain a list of indices into the vertices for this face
			int numVertices = 0;
			int vertexIndices[20];	// better not have this many!
			int indices[3];
			char number[16];

			facesCount++;
			if (facesCount % 10000 == 0)
				cout << "\t faces " << facesCount << endl;	// display in command line window faces count by 10000

			while (token != NULL)	// a token is a collection of vertex index, texture index, normal index
									// vertex index is mandatory, texture index and normal index are optional
			{
				// process one group of numbers
				int charCount;
				int indexCount = 0;
				int totalCharCount = 0;
				// copy characters into temp until '/' or blank is found
				token2 = token;		// set up start of search
				int len = strlen(token);
				indices[0] = indices[1] = indices[2] = -1;
				// first index is mandatory
				while (totalCharCount < len && token2 != NULL)
				{
					charCount = 0;
					while (*token2 != '/' && totalCharCount < len)
					{
						number[charCount++] = *token2;
						token2++;
						totalCharCount++;
					}
					number[charCount] = '\0';	// add trailing NULL
					if (totalCharCount < len)
					{
						token2++;	// move pointer past /
						totalCharCount++;
					}
					indices[indexCount++] = atoi(number);
					if (totalCharCount < len && *token2 == '/') // two slashes in a row
					{
						token2++;
						totalCharCount++;
						indices[indexCount++] = -1;	// place holder for no texture id
					}
				}

				// save the vertex indices as we process them
				vertexIndices[numVertices++] = indices[0];

				// take this 'face' description and put values into the CPU arrays that will be used to
				// fill the OpenGL VBOs

				// store the position information into app buffer
				vPosition.push_back(vertices[indices[0] - 1].x);
				vPosition.push_back(vertices[indices[0] - 1].y);
				vPosition.push_back(vertices[indices[0] - 1].z);

				totalVertexCount++;		// total number of vertices into this app buffer

				// store the currently selected colors into app buffer
				vAmbientColor.push_back(ambientSelected[0]);
				vAmbientColor.push_back(ambientSelected[1]);
				vAmbientColor.push_back(ambientSelected[2]);
				vDiffuseColor.push_back(diffuseSelected[0]);
				vDiffuseColor.push_back(diffuseSelected[1]);
				vDiffuseColor.push_back(diffuseSelected[2]);

				// store the normal information if specified
				if (indices[2] != -1)
				{
					vNormal.push_back(normals[indices[2] - 1].x);
					vNormal.push_back(normals[indices[2] - 1].y);
					vNormal.push_back(normals[indices[2] - 1].z);
					computeNormals = false;
				}
				else
				{
					computeNormals = true;
				}

				token = strtok(NULL, " ");
			}	// processing all tokens

			// if normals were not specified, then compute one now

			if (computeNormals)
			{
				float v1x = vertices[vertexIndices[1] - 1].x - vertices[vertexIndices[0] - 1].x;
				float v1y = vertices[vertexIndices[1] - 1].y - vertices[vertexIndices[0] - 1].y;
				float v1z = vertices[vertexIndices[1] - 1].z - vertices[vertexIndices[0] - 1].z;
				float v2x = vertices[vertexIndices[2] - 1].x - vertices[vertexIndices[0] - 1].x;
				float v2y = vertices[vertexIndices[2] - 1].y - vertices[vertexIndices[0] - 1].y;
				float v2z = vertices[vertexIndices[2] - 1].z - vertices[vertexIndices[0] - 1].z;
				vec3 v1(v1x, v1y, v1z);
				vec3 v2(v2x, v2y, v2z);
				vec3 norm = cross(v1, v2);
				norm = normalize(norm);

				for (int i = 0; i<numVertices; i++)
				{
					vNormal.push_back(norm[0]);
					vNormal.push_back(norm[1]);
					vNormal.push_back(norm[2]);
				}
			}


		}	// reading f description
		else // unrecognized token, so just skip it
		{
			objFile.getline(line, 1024);
		}
	}	// while loop reading entire obj file

	// return the total number of vertices in this object, needed when drawing
	*VAOsize = totalVertexCount;

	// now transfer all the information from the CPU arrays into the OpenGL VBOs
	int sizeBuff = sizeof(vPosition);
	int numEntries = vPosition.size();
	sizeBuff = sizeof(vAmbientColor);
	numEntries = vAmbientColor.size();
	sizeBuff = sizeof(vNormal);
	numEntries = vNormal.size();

	// create OpenGL buffers for vertices, colors, and normals
	GLuint vertexPosition, vertexAmbientColor, vertexDiffuseColor, vertexNormal;
	glGenBuffers(1, &vertexPosition);
	glGenBuffers(1, &vertexAmbientColor);
	glGenBuffers(1, &vertexDiffuseColor);
	glGenBuffers(1, &vertexNormal);

	glBindBuffer(GL_ARRAY_BUFFER, vertexPosition);
	glBufferData(GL_ARRAY_BUFFER, vPosition.size() * sizeof(float), &vPosition[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexNormal);
	glBufferData(GL_ARRAY_BUFFER, vNormal.size() * sizeof(float), &vNormal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vertexAmbientColor);
	glBufferData(GL_ARRAY_BUFFER, vAmbientColor.size() * sizeof(float), &vAmbientColor[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vertexDiffuseColor);
	glBufferData(GL_ARRAY_BUFFER, vDiffuseColor.size() * sizeof(float), &vDiffuseColor[0], GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	// print out information on this obj file
	cout << "\tbounds of the file: " << bounds[0] << " x " << bounds[1] << " "
		<< bounds[2] << " y " << bounds[3] << " "
		<< bounds[4] << " z " << bounds[5] << endl;
	cout << "\tnumber of faces " << facesCount << " number of vertices " << verticesCount << endl;
	//cout << "\tcounts, number vertices " << vPosition.size() << " number colors " << vAmbientColor.size() << " and " << vDiffuseColor.size()
	//	<< " number normals " << vNormal.size() << endl;

	return 1;
}