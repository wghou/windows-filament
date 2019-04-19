#pragma once
#ifndef OBJLOADER_H
#define OBJLOADER_H

#include<string>
#include<vector>

using namespace std;

class OBJLoader
{
public:
	void loadObj(const std::string &fileName, bool gl_ccw = false);

	const vector<float> & getVertices() { return mVertices; }
	const vector<float> & getNormals() { return mNormals; }
	const vector<uint16_t> & getFaces() { return mFaces; }
	const vector<float> & getTangents() { return mTangents; }

	const int getNumVertices() { return numVertices; }
	const int getNumFaces() { return numFaces; }

	void Scale(float s);
	void Scale(float sx, float sy, float sz);
	void Rotate_X(float angle);
	void Rotate_Y(float angle);
	void Rotate_Z(float angle);

	void Center(float c[]);
	void Centralize();
	void Translate(float tx, float ty, float tz);

private:
	// Êý¾Ý»º´æ
	vector<float> mVertices, mNormals;
	vector<float> mTangents;
	vector<uint16_t> mFaces;

	int numVertices = 0;
	int numFaces = 0;
};

#endif // !OBJLOADER_H
