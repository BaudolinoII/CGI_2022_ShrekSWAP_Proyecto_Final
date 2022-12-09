#include<iostream>
#include"Donut.h"

int main() {
	const unsigned int iQ = getRealSubDiv(1), fQ = getRealSubDiv(1);
	int VTsize = (iQ + 1) * (fQ + 1), Fsize = iQ * fQ * 2;
	int* Farray = getIndexArray(iQ, fQ);
	float* UVarray = getUVArray(iQ, fQ);
	float* Varray = getVertexArray(10, 10, iQ, fQ);
	float* Narray = getNormalArray(Varray, Farray, VTsize);
	for(unsigned int i = 0; i < VTsize; i++)
		std::cout << "v " << Varray[3 * i] << " " << Varray[3 * i + 1] << " " << Varray[3 * i + 2] << std::endl;
	for (unsigned int i = 0; i < VTsize; i++)
		std::cout << "vt " << UVarray[2 * i] << " " << UVarray[2 * i + 1] << std::endl;
	for (unsigned int i = 0; i < VTsize; i++)
		std::cout << "n " << Narray[3 * i] << " " << Narray[3 * i + 1] << " " << Narray[3 * i + 2] << std::endl;
	for (unsigned int i = 0; i < Fsize; i++)
		std::cout << "f " << Farray[3 * i] + 1 << "/" << i + 1 << "/" << i + 1 << " " << Farray[3 * i + 1] + 1<< "/" << i + 1 << "/" << i + 1 << " " << Farray[3 * i + 2] + 1 << "/" << i + 1 << "/" << i + 1 << " " << std::endl;
	return 0;
}