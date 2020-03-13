//
// Created by Dmitry Kozlov on 3/6/2020.
//
#include "interfaces/IVector.h"
#include "interfaces/ILogger.h"
#include <cmath>
#include <new>
#include <cstring>
IVector::~IVector() = default;

namespace {

    class Vector_Impl : public IVector {
    public:
        Vector_Impl(size_t dim, double *pCoords);
        ~Vector_Impl() override;
        IVector* clone() const override;
        size_t getDim() const override;
        double getCoord(size_t index) const override;
        RESULT_CODE setCoord(size_t index, double value) override;
    protected:
        size_t m_dim{0};
        double *m_ptr_coord{nullptr};
        ILogger * logger;
    };
}//end Vector_Impl

Vector_Impl::Vector_Impl(size_t dim, double *pCoords): m_dim(dim), m_ptr_coord(pCoords){
    logger = ILogger::createLogger(this);
}

Vector_Impl::~Vector_Impl(){
    logger->destroyLogger(this);
};

double Vector_Impl::getCoord(size_t index) const {
    if(index + 1 > m_dim){
        logger->log("Index is out of bounds", RESULT_CODE::OUT_OF_BOUNDS);
        return NAN;
    }
    return m_ptr_coord[index];
}

size_t Vector_Impl::getDim() const {
    return this->m_dim;
}

RESULT_CODE Vector_Impl::setCoord(size_t index, double value) {
    if(index + 1 > m_dim){
        logger->log("Index is out of bounds", RESULT_CODE::OUT_OF_BOUNDS);
        return RESULT_CODE::OUT_OF_BOUNDS;
    }
    if(_isnan(value)){
        logger->log("Value is NAN", RESULT_CODE::NAN_VALUE);
        return RESULT_CODE::NAN_VALUE;
    }
    m_ptr_coord[index] = value;
    return RESULT_CODE::SUCCESS;
}

IVector* Vector_Impl::clone() const {
    return IVector::createVector(this->m_dim, this->m_ptr_coord, logger);
}

IVector* IVector::add(IVector const *pOperand1, IVector const *pOperand2, ILogger* pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Some operands are nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    if (pOperand1->getDim() != pOperand2->getDim()){
        if (pLogger != nullptr){
            pLogger->log("Different dimensions of operands", RESULT_CODE::WRONG_DIM);
        }
        return nullptr;
    }
    size_t _dim = pOperand1->getDim();
    auto * _arr = new (std::nothrow) double[_dim];
    if (!_arr){
        if (pLogger != nullptr){
            pLogger->log("Not enough memory", RESULT_CODE::OUT_OF_MEMORY);
        }
        return nullptr;
    }
    for(size_t i = 0; i < _dim; ++i){
        _arr[i] = pOperand1->getCoord(i) + pOperand2->getCoord(i);
    }
    IVector *returnValue = createVector(_dim, _arr, pLogger);
    delete[] _arr;
    return returnValue;
}

IVector* IVector::sub(IVector const *pOperand1, IVector const *pOperand2, ILogger* pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Some operands are nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    if (pOperand1->getDim() != pOperand2->getDim()){
        if (pLogger != nullptr){
            pLogger->log("Different dimensions of operands", RESULT_CODE::WRONG_DIM);
        }
        return nullptr;
    }
    size_t _dim = pOperand1->getDim();
    auto * _arr = new (std::nothrow) double[_dim];
    if (!_arr){
        if (pLogger != nullptr){
            pLogger->log("Not enough memory", RESULT_CODE::OUT_OF_MEMORY);
        }
        return nullptr;
    }
    for(size_t i = 0; i < _dim; ++i){
        _arr[i] = pOperand1->getCoord(i) - pOperand2->getCoord(i);
    }
    IVector *returnValue = createVector(_dim, _arr, pLogger);
    delete[] _arr;
    return returnValue;
}

double IVector::mul(IVector const *pOperand1, IVector const *pOperand2, ILogger* pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Some operands are nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return NAN;
    }
    if (pOperand1->getDim() != pOperand2->getDim()){
        if (pLogger != nullptr){
            pLogger->log("Different dimensions of operands", RESULT_CODE::WRONG_DIM);
        }
        return NAN;
    }
    size_t _dim = pOperand1->getDim();
    double ans = 0;
    for(size_t i = 0; i < _dim; ++i){
        double val = pOperand1->getCoord(i) * pOperand2->getCoord(i);
        if (_isnan(val)){
            if (pLogger != nullptr){
                pLogger->log("Result of mul is NAN", RESULT_CODE::CALCULATION_ERROR);
            }
            return NAN;
        }
        ans += val;
    }
    return ans;
}

IVector* IVector::mul(IVector const *pOperand1, double scaleParam, ILogger* pLogger) {
    if (pOperand1 == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Operand is nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    if(_isnan(scaleParam)){
        if (pLogger != nullptr){
            pLogger->log("Scale param is NAN", RESULT_CODE::NAN_VALUE);
        }
        return nullptr;
    }
    size_t _dim = pOperand1->getDim();
    auto* _arr = new (std::nothrow) double[_dim];
    if (!_arr){
        if (pLogger != nullptr){
            pLogger->log("Not enough memory", RESULT_CODE::OUT_OF_MEMORY);
        }
        return nullptr;
    }
    for(size_t i = 0; i < _dim; ++i){
        _arr[i] = pOperand1->getCoord(i) * scaleParam;
        if (_isnan(_arr[i])){
            if (pLogger != nullptr){
                pLogger->log("Result of mul is NAN", RESULT_CODE::CALCULATION_ERROR);
            }
            return nullptr;
        }
    }
    IVector *returnValue = createVector(_dim, _arr, pLogger);
    delete[] _arr;
    return returnValue;
}


IVector* IVector::createVector(size_t dim, double *pData, ILogger* pLogger) {
    if(!dim){
        if (pLogger != nullptr){
            pLogger->log("Dimension must be more than 0", RESULT_CODE::WRONG_DIM);
        }
        return nullptr;
    }
    if(pData == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Data array is nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    for(size_t i = 0; i < dim; ++i){
        if(_isnan(pData[i])){
            if (pLogger != nullptr){
                pLogger->log("Some numbers in data array are NAN", RESULT_CODE::NAN_VALUE);
            }
            return nullptr;
        }
    }
    size_t _size = sizeof(Vector_Impl) + dim * sizeof(double);
    void *ptr = new(std::nothrow) unsigned char[_size];
    if (!ptr){
        if (pLogger != nullptr){
            pLogger->log("Not enough memory", RESULT_CODE::OUT_OF_MEMORY);
        }
        return nullptr;
    }
    auto *pVector = new(ptr)Vector_Impl(dim,
                                               reinterpret_cast<double *>((unsigned char *) ptr + sizeof(Vector_Impl)));
    memcpy((unsigned char*)ptr + sizeof(Vector_Impl), pData, dim * sizeof(double));
    return pVector;
}

double IVector::norm(IVector const *pVector, IVector::NORM norm, ILogger *pLogger) {
    if (pVector == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Vector is nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return NAN;
    }
    double ans = 0;
    switch (norm){
        case IVector::NORM::NORM_1:
            for (size_t i = 0; i < pVector->getDim(); ++i){
                ans += fabs(pVector->getCoord(i));
            }
            break;
        case IVector::NORM::NORM_2:
            double val;
            for (size_t i = 0; i < pVector->getDim(); ++i){
                val = pVector->getCoord(i)*pVector->getCoord(i);
                if (_isnan(val)){
                    if(pLogger != nullptr){
                        pLogger->log("Result of mul is NAN", RESULT_CODE::CALCULATION_ERROR);
                    }
                    return NAN;
                }
                ans += val;
            }
            ans = sqrt(ans);
            break;
        case IVector::NORM::NORM_INF:
            double cur;
            ans = fabs(pVector->getCoord(0));
            for (size_t i = 1; i < pVector->getDim(); ++i){
                cur = fabs(pVector->getCoord(i));
                ans = cur > ans ? cur : ans;
            }
            break;
        default:
            if (pLogger != nullptr){
                pLogger->log("Unknown type of norm", RESULT_CODE::WRONG_ARGUMENT);
            }
            return NAN;
    }
    return ans;
}

RESULT_CODE IVector::equals(IVector const *pOperand1, IVector const *pOperand2, IVector::NORM norm, double tolerance,
                            bool *result, ILogger *pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Some operands are nullptr", RESULT_CODE::BAD_REFERENCE);
        }
        return RESULT_CODE::BAD_REFERENCE;
    }
    if (_isnan(tolerance)){
        if (pLogger != nullptr){
            pLogger->log("Tolerance is NAN", RESULT_CODE::NAN_VALUE);
        }
        return RESULT_CODE::NAN_VALUE;
    }
    IVector *subVectors = IVector::sub(pOperand1, pOperand2, pLogger);
    if (subVectors == nullptr){
        if (pLogger != nullptr){
            pLogger->log("Wrong calculating of sub", RESULT_CODE::CALCULATION_ERROR);
        }
        return RESULT_CODE::CALCULATION_ERROR;
    }
    double normValue = IVector::norm(subVectors, norm, pLogger);
    if (_isnan(normValue)){
        if (pLogger != nullptr){
            pLogger->log("Norm's value is NAN", RESULT_CODE::NAN_VALUE);
        }
        return RESULT_CODE::NAN_VALUE;
    }
    *result = normValue <= tolerance;
    delete subVectors;
    return RESULT_CODE::SUCCESS;
}