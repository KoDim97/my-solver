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
        Vector_Impl(size_t dim, double *pCoords, ILogger* pLogger);
        ~Vector_Impl() override;
        IVector* clone() const override;
        size_t getDim() const override;
        double getCoord(size_t index) const override;
        RESULT_CODE setCoord(size_t index, double value) override;
        double norm(NORM norm) const override;
    protected:
        size_t m_dim{0};
        double *m_ptr_coord{nullptr};
        ILogger * logger {nullptr};
    };
}//end Vector_Impl

Vector_Impl::Vector_Impl(size_t dim, double *pCoords, ILogger* pLogger): m_dim(dim), m_ptr_coord(pCoords), logger(pLogger){}

Vector_Impl::~Vector_Impl(){};

double Vector_Impl::getCoord(size_t index) const {
    if(index + 1 > m_dim){
        logger->log("In getCoord(...)", RESULT_CODE::OUT_OF_BOUNDS);
        return NAN;
    }
    return m_ptr_coord[index];
}

size_t Vector_Impl::getDim() const {
    return this->m_dim;
}

RESULT_CODE Vector_Impl::setCoord(size_t index, double value) {
    if(index + 1 > m_dim){
        logger->log("In setCoord(...)", RESULT_CODE::OUT_OF_BOUNDS);
        return RESULT_CODE::OUT_OF_BOUNDS;
    }
    if(__isnan(value)){
        logger->log("In setCoord(...)", RESULT_CODE::NAN_VALUE);
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
            pLogger->log("In add(...)", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    if (pOperand1->getDim() != pOperand2->getDim()){
        if (pLogger != nullptr){
            pLogger->log("In add(...) expected the same dim of operands", RESULT_CODE::WRONG_DIM);
        }
        return nullptr;
    }
    size_t _dim = pOperand1->getDim();
    auto * _arr = new (std::nothrow) double[_dim];
    if (!_arr){
        if (pLogger != nullptr){
            pLogger->log("In add(...)", RESULT_CODE::OUT_OF_MEMORY);
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
            pLogger->log("In sub(...)", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    if (pOperand1->getDim() != pOperand2->getDim()){
        if (pLogger != nullptr){
            pLogger->log("In sub(...) expected the same dim of operands", RESULT_CODE::WRONG_DIM);
        }
        return nullptr;
    }
    size_t _dim = pOperand1->getDim();
    auto * _arr = new (std::nothrow) double[_dim];
    if (!_arr){
        if (pLogger != nullptr){
            pLogger->log("In sub(...)", RESULT_CODE::OUT_OF_MEMORY);
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
            pLogger->log("In mul(...)", RESULT_CODE::BAD_REFERENCE);
        }
        return NAN;
    }
    if (pOperand1->getDim() != pOperand2->getDim()){
        if (pLogger != nullptr){
            pLogger->log("In mul(...) expected the same dim of operands", RESULT_CODE::WRONG_DIM);
        }
        return NAN;
    }
    size_t _dim = pOperand1->getDim();
    double ans = 0;
    for(size_t i = 0; i < _dim; ++i){
        double val = pOperand1->getCoord(i) * pOperand2->getCoord(i);
        if (__isnan(val)){
            if (pLogger != nullptr){
                pLogger->log("In mul(...)", RESULT_CODE::CALCULATION_ERROR);
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
            pLogger->log("In mul(...)", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }
    if(__isnan(scaleParam)){
        if (pLogger != nullptr){
            pLogger->log("Scale param in mul(...)", RESULT_CODE::NAN_VALUE);
        }
        return nullptr;
    }
    size_t _dim = pOperand1->getDim();
    auto* _arr = new (std::nothrow) double[_dim];
    if (!_arr){
        if (pLogger != nullptr){
            pLogger->log("In mul(...)", RESULT_CODE::OUT_OF_MEMORY);
        }
        return nullptr;
    }
    for(size_t i = 0; i < _dim; ++i){
        _arr[i] = pOperand1->getCoord(i) * scaleParam;
        if (__isnan(_arr[i])){
            if (pLogger != nullptr){
                pLogger->log("In mul(...)", RESULT_CODE::CALCULATION_ERROR);
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
            pLogger->log("In createVetor(...) dimension must be more than 0", RESULT_CODE::WRONG_DIM);
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
        if(__isnan(pData[i])){
            if (pLogger != nullptr){
                pLogger->log("In data array", RESULT_CODE::NAN_VALUE);
            }
            return nullptr;
        }
    }
    size_t _size = sizeof(Vector_Impl) + dim * sizeof(double);
    void *ptr = new(std::nothrow) unsigned char[_size];
    if (!ptr){
        if (pLogger != nullptr){
            pLogger->log("In createVector(...)", RESULT_CODE::OUT_OF_MEMORY);
        }
        return nullptr;
    }
    auto *pVector = new(ptr)Vector_Impl(dim,
                                               reinterpret_cast<double *>((unsigned char *) ptr + sizeof(Vector_Impl)), pLogger);
    memcpy((unsigned char*)ptr + sizeof(Vector_Impl), pData, dim * sizeof(double));
    return pVector;
}

double Vector_Impl::norm(IVector::NORM norm) const {
    double ans = 0;
    switch (norm){
        case IVector::NORM::NORM_1:
            for (size_t i = 0; i < this->getDim(); ++i){
                ans += fabs(this->getCoord(i));
            }
            break;
        case IVector::NORM::NORM_2:
            double val;
            for (size_t i = 0; i < this->getDim(); ++i){
                val = this->getCoord(i)*this->getCoord(i);
                if (__isnan(val)){
                    if(logger != nullptr){
                        logger->log("In norm(...)", RESULT_CODE::CALCULATION_ERROR);
                    }
                    return NAN;
                }
                ans += val;
            }
            ans = sqrt(ans);
            break;
        case IVector::NORM::NORM_INF:
            double cur;
            ans = fabs(this->getCoord(0));
            for (size_t i = 1; i < this->getDim(); ++i){
                cur = fabs(this->getCoord(i));
                ans = cur > ans ? cur : ans;
            }
            break;
        default:
            if (logger != nullptr){
                logger->log("In norm(...) unknown type of norm", RESULT_CODE::WRONG_ARGUMENT);
            }
            return NAN;
    }
    return ans;
}

RESULT_CODE IVector::equals(IVector const *pOperand1, IVector const *pOperand2, IVector::NORM norm, double tolerance,
                            bool *result, ILogger *pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr){
        if (pLogger != nullptr){
            pLogger->log("In equals(...)", RESULT_CODE::BAD_REFERENCE);
        }
        return RESULT_CODE::BAD_REFERENCE;
    }
    if (__isnan(tolerance)){
        if (pLogger != nullptr){
            pLogger->log("In equals(...) tolerance is NAN", RESULT_CODE::NAN_VALUE);
        }
        return RESULT_CODE::NAN_VALUE;
    }
    IVector *subVectors = IVector::sub(pOperand1, pOperand2, pLogger);
    if (subVectors == nullptr){
        if (pLogger != nullptr){
            pLogger->log("In equals(...) wrong calculating of sub", RESULT_CODE::BAD_REFERENCE);
        }
        return RESULT_CODE::CALCULATION_ERROR;
    }
    double normValue = subVectors->norm(norm);
    if (__isnan(normValue)){
        if (pLogger != nullptr){
            pLogger->log("In equals(...) norm's value is NAN", RESULT_CODE::NAN_VALUE);
        }
        return RESULT_CODE::NAN_VALUE;
    }
    *result = normValue <= tolerance;
    delete subVectors;
    return RESULT_CODE::SUCCESS;
}