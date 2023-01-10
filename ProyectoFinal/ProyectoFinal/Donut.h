#ifndef DONUT_CLASS_H
#define DONUT_CLASS_H

#include<vector>

unsigned int getRealSubDiv(const int n);
void constantCircle(std::vector<float>* vec, const float radius, const unsigned int inQual);

void getIndexArray(std::vector<int>* vec,const unsigned int IQ, const unsigned int FQ);
void getVertexArray(std::vector<float>* vec,const float inRad, const float forRad, const unsigned int IQ, const unsigned int FQ);
#endif // DONUT_CLASS_H