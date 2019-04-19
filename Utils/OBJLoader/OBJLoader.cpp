#include"OBJLoader.h"
#include<sstream>
#include<fstream>
#include<iostream>
#include<vector>


#include <math/norm.h>
#include <math/vec3.h>
#include <math/quat.h>
#include <math/mat3.h>
#include <math/TVecHelpers.h>

using namespace std;

void OBJLoader::loadObj(const std::string &fileName, bool gl_ccw)
{
	mVertices.clear(); mNormals.clear(); mFaces.clear(); mTangents.clear();
	numVertices = 0; numFaces = 0;

	//cout << "Loading " << fileName << endl;

	ifstream fileStream;
	fileStream.open(fileName.c_str());
	if (fileStream.fail())
	{
		std::cerr << "Failed to open file: " << fileName << endl;
		return;
	}

	string line_stream;
	bool vn = false;

	while (getline(fileStream,line_stream))
	{
		stringstream str_stream(line_stream);
		string type_str;
		str_stream >> type_str;

		if (type_str == "v")
		{
			// 节点坐标
			float temp1, temp2, temp3;
			str_stream >> temp1 >> temp2 >> temp3;
			mVertices.push_back(temp1);
			mVertices.push_back(temp2);
			mVertices.push_back(temp3);

			numVertices++;
		}
		else if (type_str == "vt")
		{
			// 忽略纹理坐标
		}
		else if (type_str == "vn")
		{
			// 节点法向量
			float temp1, temp2, temp3;
			str_stream >> temp1 >> temp2 >> temp3;
			mNormals.push_back(temp1);
			mNormals.push_back(temp2);
			mNormals.push_back(temp3);
			vn = true;
		}
		else if (type_str == "f")
		{
			// 三角形面片
			if (vn == false)
			{
				uint16_t temp1, temp2, temp3;
				str_stream >> temp1 >> temp2 >> temp3;

				if (gl_ccw)
				{
					mFaces.push_back(temp1 - 1);
					mFaces.push_back(temp3 - 1);
					mFaces.push_back(temp2 - 1);
				}
				else
				{
					mFaces.push_back(temp1 - 1);
					mFaces.push_back(temp2 - 1);
					mFaces.push_back(temp3 - 1);
				}
			}
			else
			{
				string temp1[3];
				uint16_t temp2[3];
				str_stream >> temp1[0] >> temp1[1] >> temp1[2];

				// 格式为：index//index index//index index//index
				for (int i = 0; i < 3; i++)
				{
					string temp3 = temp1[i].substr(0, temp1[i].length() / 2 - 1);
					temp2[i] = stoi(temp3);
				}

				if (gl_ccw)
				{
					mFaces.push_back(temp2[0] - 1);
					mFaces.push_back(temp2[2] - 1);
					mFaces.push_back(temp2[1] - 1);
				}
				else
				{
					mFaces.push_back(temp2[0] - 1);
					mFaces.push_back(temp2[1] - 1);
					mFaces.push_back(temp2[2] - 1);
				}
			}

			numFaces++;
		}
	}

	using namespace filament::math;
	using namespace filament::math::details;
	// calculate the tangents
	for (int i = 0; i < numVertices; i++)
	{
		float3 normal = float3{ mNormals[i * 3 + 0], mNormals[i * 3 + 1], mNormals[i * 3 + 2] };
		float3 tangent;
		float3 bitangent;

		// calculate tangent
		bitangent = normalize(cross(normal, float3{ 1.0, 0.0, 0.0 }));
		tangent = normalize(cross(normal, bitangent));

		// calculate the quaternion
		//quatf q = filament::math::details::TMat33<float>::packTangentFrame({ tangent, bitangent, normal });
		//short4 qs4 = packSnorm16(q.xyzw);

		short4 qs4 = filament::math::packSnorm16(mat3f::packTangentFrame(mat3f{ tangent,bitangent, normal }).xyzw);

		// store
		mTangents.push_back(qs4.x);
		mTangents.push_back(qs4.y);
		mTangents.push_back(qs4.z);
		mTangents.push_back(qs4.w);
	}

	return;
}


void OBJLoader::Scale(float s)
{
	for (vector<float>::iterator it = mVertices.begin(); it != mVertices.end(); it++)
	{
		(*it) *= s;
	}
}


void OBJLoader::Scale(float sx, float sy, float sz)
{
	int number = mVertices.size() / 3;
	if (number * 3 != mVertices.size()) return;


	for (int i = 0; i < number; i++)
	{
		mVertices[3 * i + 0] *= sx;
		mVertices[3 * i + 1] *= sy;
		mVertices[3 * i + 2] *= sz;
	}
}


void OBJLoader::Rotate_X(float angle)
{
	int number = mVertices.size() / 3;
	if (number * 3 != mVertices.size()) return;


	for (int i = 0; i < number; i++)
	{
		float y = mVertices[i * 3 + 1];
		float z = mVertices[i * 3 + 2];
		mVertices[i * 3 + 1] = y*cos(angle) + z*sin(angle);
		mVertices[i * 3 + 2] = -y*sin(angle) + z*cos(angle);
	}
}


void OBJLoader::Rotate_Y(float angle)
{
	int number = mVertices.size() / 3;
	if (number * 3 != mVertices.size()) return;


	for (int i = 0; i < number; i++)
	{
		float x = mVertices[i * 3 + 0];
		float z = mVertices[i * 3 + 2];
		mVertices[i * 3 + 0] = x*cos(angle) + z*sin(angle);
		mVertices[i * 3 + 2] = -x*sin(angle) + z*cos(angle);
	}
}


void OBJLoader::Rotate_Z(float angle)
{
	int number = mVertices.size() / 3;
	if (number * 3 != mVertices.size()) return;


	for (int i = 0; i < number; i++)
	{
		float x = mVertices[i * 3 + 0];
		float y = mVertices[i * 3 + 1];
		mVertices[i * 3 + 0] = x*cos(angle) + y*sin(angle);
		mVertices[i * 3 + 1] = -x*sin(angle) + y*cos(angle);
	}
}


void OBJLoader::Center(float c[])
{
	int number = mVertices.size() / 3;
	if (number * 3 != mVertices.size()) return;

	c[0] = c[1] = c[2] = 0;
	float mass_sum = 0;
	for (int i = 0; i<number; i++)
	{
		c[0] += mVertices[i * 3 + 0];
		c[1] += mVertices[i * 3 + 1];
		c[2] += mVertices[i * 3 + 2];
		mass_sum += 1;
	}
	c[0] /= mass_sum;
	c[1] /= mass_sum;
	c[2] /= mass_sum;
}


void OBJLoader::Centralize()
{
	float c[3];
	Center(c);
	printf("Centralize: x: %f,  y: %f,  z: %f", c[0], c[1], c[2]);
	Translate(-c[0], -c[1], -c[2]);
}


void OBJLoader::Translate(float tx, float ty, float tz)
{
	int number = mVertices.size() / 3;
	if (number * 3 != mVertices.size()) return;

	for (int i = 0; i<number; i++)
	{
		mVertices[i * 3 + 0] += tx;
		mVertices[i * 3 + 1] += ty;
		mVertices[i * 3 + 2] += tz;
	}
}
