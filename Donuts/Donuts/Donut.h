#ifndef DONUT_CLASS_H
#define DONUT_CLASS_H

unsigned int getRealSubDiv(const int n);
float* constantCircle(const float radius, const unsigned int inQual);

int* getIndexArray(const unsigned int inQual, const unsigned int forQual);
float* getUVArray(const unsigned int inQual, const unsigned int forQual);
float* getVertexArray(const float inRad, const float forRad, const unsigned int inQual, const unsigned int forQual);
float* getNormalArray(float* vertexArray, int* indexArray, const unsigned int size);
#endif // DONUT_CLASS_H