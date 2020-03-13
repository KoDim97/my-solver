#include <iostream>
#include "interfaces/IVector.h"
#include <math.h>

int main() {
    double arr1[2] = {1,2};
    double arr2[2] = {3,2};
    ILogger *logger = ILogger::createLogger(nullptr);
    IVector *vec1 = IVector::createVector(2, arr1, logger);
    IVector *vec2 = IVector::createVector(2, arr2, logger);
    IVector *vec3 = vec1->clone();
    IVector *vec4 = IVector::add(vec1, vec2, logger);
    IVector *vec5 = IVector::mul(vec1, 2.0, logger);
    delete vec1;
    delete vec2;
    delete vec3;
    delete vec4;
    delete vec5;
    logger->destroyLogger(nullptr);
    return 0;
}