#include"Donut.h"
#include<cmath>
#include<fstream>
#include<iostream>
#include<vector>


#define PI 3.14159265358979323846f


unsigned int getRealSubDiv(const int n) {
	if (n < 1)
		return 2;
	return 2*n + 2;
}
void constantCircle(std::vector<float>* vec, const float radius, const unsigned int IQ) {
	vec->resize(IQ);
	const float incX = PI * 2 / (float)IQ;
	for (int i = 0; i < IQ; i++) {
		const float zeta = radius * (float)std::sin(incX * (float)i);
		 vec->at(i) = 0.00001 < abs(zeta) ? zeta : 0.0f;
	}
}

void getIndexArray(std::vector<int>* vec, const unsigned int IQ, const unsigned int FQ) {
	vec->resize((IQ * FQ - 1) * 6);
	//std::cout << "Tamanio Index Array = " << vec->size() << std::endl;
	int nx = 0, ny = 0, nz = 0, seq = 0;
	for (int i = 0; i < FQ; i++) 
		for (int j = 0; j < IQ; j++) 
			if(j != (IQ - 1) || i != (FQ - 1)) {
				nx = i * IQ + j;
				nz = ( i + 1 == FQ ? j : nx + IQ );
				ny = ( j + 1 == IQ ? IQ * (i + 1) :nz + 1); 
			
				vec->at(i * IQ + j) = nx;
				vec->at(i * IQ + j + 1) = ny;
				vec->at(i * IQ + j + 2) = nz;
				vec->at(i * IQ + j + 3) = (nx + 1) % (IQ * (i + 1)) + (j + 1 == IQ ? IQ * i : 0);
				vec->at(i * IQ + j + 4) = ny;
				vec->at(i * IQ + j + 5) = nx;
				std::cout << vec->at(i * IQ + j) << ", " << vec->at(i * IQ + j + 1) << ", " << vec->at(i * IQ + j + 2) << "," << std::endl;
				std::cout << vec->at(i * IQ + j + 3) << ", " << vec->at(i * IQ + j + 4) << ", " << vec->at(i * IQ + j + 5) << ", " << std::endl;
			}
}
void getVertexArray(std::vector<float>* vec, const float inRad, const float forRad, const unsigned int IQ, const unsigned int FQ) {
	vec->resize(IQ * FQ * 3);
	//std::cout << "Tamanio Vertex Array = " << vec->size() << std::endl;
	const float incAng = 2 * PI / (float)FQ;
	const float incIR = inRad / (float)IQ;
	std::vector<float> inCircle;
	constantCircle(&inCircle, inRad, IQ);
	for (unsigned int i = 0; i < FQ; i++) {
		float pivX = std::cos(incAng * (float)i);
		float pivZ = std::sin(incAng * (float)i);
		pivX = (0.00001 < abs(pivX) ? pivX : 0.0f); pivZ = (0.00001 < abs(pivZ) ? pivZ : 0.0f);
		for (unsigned int j = 0; j < IQ; j++) {
			vec->at(i * IQ + j * 3) = (forRad + j * incIR) * pivX;
			vec->at(i * IQ + j * 3 + 1) = inCircle.at(j);
			vec->at(i * IQ + j * 3 + 2) = (forRad + j * incIR) * pivZ;
			std::cout << vec->at(i * IQ + j * 3) << "f, " << vec->at(i * IQ + j * 3 + 1) << "f, " << vec->at(i * IQ + j * 3 + 2) << "f," << std::endl;
		}
	}
}
