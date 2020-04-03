#include <iostream>
#include "interfaces/IVector.h"
#include "interfaces/ISet.h"

int main() {
    double arr1[2] = {1,2};
    double arr2[2] = {3,2};
    ILogger *logger = ILogger::createLogger(nullptr);
    logger->log("hah", RESULT_CODE::SUCCESS);
    IVector *vec1 = IVector::createVector(2, arr1, logger);
    IVector *vec2 = IVector::createVector(2, arr2, logger);
    IVector *vec3 = vec1->clone();
    IVector *vec4 = IVector::add(vec1, vec2, logger);
    IVector *vec5 = IVector::mul(vec1, 2.0, logger);
    ISet *set1 = ISet::createSet(logger);
    set1->insert(vec1, IVector::NORM::NORM_1, 0.01);
    set1->insert(vec2, IVector::NORM::NORM_1, 0.01);
    set1->insert(vec3, IVector::NORM::NORM_2, 0.01);
    ISet *set2 = set1->clone();
    ISet *unionSet = ISet::add(set1, set2, IVector::NORM::NORM_1, 0.01, logger);
    ISet *set3 = ISet::createSet(logger);
    set3->insert(vec3, IVector::NORM::NORM_2, 0.01);
    ISet *intersectSet = ISet::intersect(set1, set3, IVector::NORM::NORM_1, 0.01, logger);
    delete vec1;
    delete vec2;
    delete vec3;
    delete vec4;
    delete vec5;
    delete set1;
    delete set2;
    delete set3;
    logger->destroyLogger(nullptr);
    return 0;
}