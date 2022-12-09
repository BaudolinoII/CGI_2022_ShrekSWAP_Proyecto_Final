#include"Donut.h"
#include<cmath>

#define PI 3.14159265358979323846f


unsigned int getRealSubDiv(const int n) {
	if (n < 1)
		return 2;
	return 2*n + 2;
}
float* constantCircle(const float radius, const unsigned int inQual) {
	float* arrayCircle = new float[inQual];
	const float incX = PI * 2 / (float)(inQual-1);
	for (int i = 0; i < inQual; i++) {
		const float zeta = radius * (float)std::sin(incX * (float)i);
		arrayCircle[i] = (0.00001 < abs(zeta) ? zeta : 0.0f);
	}
	return arrayCircle;
}

int* getIndexArray(const unsigned int inQual, const unsigned int forQual) {
	int* indexArray = new int[inQual * forQual * 6];
	int nx = 0, ny = 0, nz = 0;
	for (int i = 0; i < forQual; i++) 
		for (int j = 0; j < inQual; j++) {
			nx = i * (inQual + 1) + j; nz = nx + (inQual + 1); ny = nz + 1;
			indexArray[(i * inQual + j) * 6] = nx; indexArray[(i * inQual + j) * 6 + 1] = ny; indexArray[(i * inQual + j) * 6 + 2] = nz;
			indexArray[(i * inQual + j) * 6 + 3] = nx + 1; indexArray[(i * inQual + j) * 6 + 4] = ny; indexArray[(i * inQual + j) * 6 + 5] = nx;
			//std::cout << nx << " " << ny << " " << nz << " " << std::endl;
			//std::cout << nx + 1<< " " << ny << " " << nx << " " << std::endl;
		}
	return indexArray;
}
float* getUVArray(const unsigned int inQual, const unsigned int forQual) {
	float* UVarray = new float[(inQual + 1) * (forQual + 1) * 2];
	const float inFact = 1.0f / (float)(inQual);
	const float forFact = 1.0f / (float)(forQual);
	for (unsigned int i = 0; i <= forQual; i++)
		for (unsigned int j = 0; j <= inQual; j++) {
			UVarray[i * (2 * inQual + 2) + j * 2] = (float)i * inFact; UVarray[i * (2 * inQual + 2) + j * 2 + 1] = (float)j * forFact;
		}
	return UVarray;
}
float* getVertexArray(const float inRad, const float forRad, const unsigned int inQual, const unsigned int forQual) {
	const unsigned int iQ = getRealSubDiv(inQual) + 1, fQ = getRealSubDiv(forQual);
	float* vertexArray = new float[iQ * (fQ + 1) * 3];
	const float incAng = 2 * PI / (float)fQ;
	float* inCircle = constantCircle(inRad, iQ);
	for (unsigned int i = 0; i <= fQ; i++) {
		float pivX = forRad * std::cos(incAng * (float)i), pivZ = forRad * std::sin(incAng * (float)i);
		pivX = (0.00001 < abs(pivX) ? pivX : 0.0f); pivZ = (0.00001 < abs(pivZ) ? pivZ : 0.0f);
		for (unsigned int j = 0; j < iQ; j++) {
			vertexArray[i * iQ + j * 3] = pivX;
			vertexArray[i * iQ + j * 3 + 1] = inCircle[j];
			vertexArray[i * iQ + j * 3 + 2] = pivZ;
		}
	}
	return vertexArray;
}
float* getNormalArray(float* vertexArray, int* indexArray, const unsigned int size) {
	float* normalArray = new float[3 * size];
	int n1, n2, n3; float vax, vay, vaz, vbx, vby, vbz, nx, ny, nz, c;
	for (unsigned int i = 0; i < size; i++) {
		n1 = 3 * indexArray[3 * i]; n2 = 3 * indexArray[3 * i + 1]; n3 = 3 * indexArray[3 * i + 2];
		vax = vertexArray[n2] - vertexArray[n1]; vay = vertexArray[n2 + 1] - vertexArray[n1 + 1]; vaz = vertexArray[n2 + 2] - vertexArray[n1 + 2];
		vbx = vertexArray[n3] - vertexArray[n1]; vby = vertexArray[n3 + 1] - vertexArray[n1 + 1]; vbz = vertexArray[n3 + 2] - vertexArray[n1 + 2];
		nx = vay * vbz - vby * vaz; ny = vax * vbz - vbx * vaz; nz = vax * vby - vbx * vay;
		c = sqrt(nx * nx + ny * ny + nz * nz); c = (c < 1.0f ? 1.0f : c);
		nx /= c; nx = (0.00001f < abs(nx) ? nx : 0.0f); ny /= c; ny = (0.00001f < abs(ny) ? ny : 0.0f); nz /= c; nz = (0.00001f < abs(nz) ? nz : 0.0f);
		normalArray[3 * i] = nx; normalArray[3 * i + 1] = ny; normalArray[3 * i + 2] = nz;
	}
	return normalArray;
}
