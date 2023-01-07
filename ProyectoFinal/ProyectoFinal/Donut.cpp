#include"Donut.h"
#include<cmath>
#include<fstream>
#include<iostream>

#define PI 3.14159265358979323846f


unsigned int getRealSubDiv(const int n) {
	if (n < 1)
		return 2;
	return 2*n + 2;
}
float* constantCircle(const float radius, const unsigned int IQ) {
	float* arrayCircle = new float[IQ];
	const float incX = PI * 2 / (float)IQ;
	for (int i = 0; i < IQ; i++) {
		const float zeta = radius * (float)std::sin(incX * (float)i);
		arrayCircle[i] = (0.00001 < abs(zeta) ? zeta : 0.0f);
	}
	return arrayCircle;
}

int* getIndexArray(const unsigned int IQ, const unsigned int FQ) {
	int* indexArray = new int[(IQ * FQ - 1) * 6];
	int nx = 0, ny = 0, nz = 0, seq = 0;
	for (int i = 0; i < FQ; i++) 
		for (int j = 0; j < IQ; j++) 
			if(j != (IQ - 1) || i != (FQ - 1)) {
				nx = i * IQ + j;
				nz = ( i + 1 == FQ ? j : nx + IQ );
				ny = ( j + 1 == IQ ? IQ * (i + 1) :nz + 1); 
			
				seq = (i * IQ + j) * 6;
				indexArray[seq] = nx; 
				indexArray[seq + 1] = ny; 
				indexArray[seq + 2] = nz;
				indexArray[seq + 3] = (nx + 1) % (IQ * (i + 1)) + (j + 1 == IQ ? IQ * i : 0);
				indexArray[seq + 4] = ny; 
				indexArray[seq + 5] = nx;
				//std::cout <<"f " << indexArray[seq] << " " << indexArray[seq + 1] << " " << indexArray[seq + 2] << " " << std::endl;
				//std::cout <<"f " << indexArray[seq + 3] << " " << indexArray[seq + 4] << " " << indexArray[seq + 5] << " " << std::endl;
			}
	return indexArray;
}
float* getVertexArray(const float inRad, const float forRad, const unsigned int IQ, const unsigned int FQ) {
	float* vertexArray = new float[IQ * FQ * 3];
	const float incAng = 2 * PI / (float)FQ;
	const float incIR = inRad / (float)IQ;
	float* inCircle = constantCircle(inRad, IQ);
	for (unsigned int i = 0; i < FQ; i++) {
		float pivX = std::cos(incAng * (float)i);
		float pivZ = std::sin(incAng * (float)i);
		pivX = (0.00001 < abs(pivX) ? pivX : 0.0f); pivZ = (0.00001 < abs(pivZ) ? pivZ : 0.0f);
		for (unsigned int j = 0; j < IQ; j++) {
			vertexArray[i * IQ + j * 3] = (forRad + j * incIR) * pivX;
			vertexArray[i * IQ + j * 3 + 1] = inCircle[j];
			vertexArray[i * IQ + j * 3 + 2] = (forRad + j * incIR) * pivZ;
			//std::cout << "v " << vertexArray[i * IQ + j * 3] << " " << vertexArray[i * IQ + j * 3 + 1] << " " << vertexArray[i * IQ + j * 3 + 2] << std::endl;
		}
	}
	return vertexArray;
}
