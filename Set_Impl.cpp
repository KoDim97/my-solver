//
// Created by Dmitry Kozlov on 3/21/2020.
//

#include "interfaces/ISet.h"
#include <vector>

ISet::~ISet() = default;

namespace {
    class Set_Impl : public ISet{
    public:
        explicit Set_Impl(ILogger* pLogger);
        ~Set_Impl() override;
        RESULT_CODE insert(const IVector* pVector,IVector::NORM norm, double tolerance) override;
        RESULT_CODE get(IVector*& pVector, size_t index) const override;
        RESULT_CODE get(IVector*& pVector, IVector const *pSample, IVector::NORM norm, double tolerance) const override;
        size_t getIndex(IVector const *pSample, IVector::NORM norm, double tolerance) const;
        size_t getDim() const override;
        size_t getSize() const override;
        void clear() override;
        RESULT_CODE erase(size_t index) override;
        RESULT_CODE erase(IVector const* pSample, IVector::NORM norm, double tolerance) override;
        ISet* clone() const override;
        friend ISet* ISet::add(ISet const* pOperand1, ISet const* pOperand2, IVector::NORM norm, double tolerance, ILogger* pLogger);
        friend ISet* ISet::intersect(ISet const *pOperand1, ISet const *pOperand2, IVector::NORM norm, double tolerance,
                                     ILogger *pLogger);
        friend ILogger* ILogger::createLogger(void *pClient);
    protected:
        std::vector<const IVector*> container;
        size_t dim;
        ILogger * logger {nullptr};
    };

    Set_Impl::Set_Impl(ILogger *pLogger) : logger(pLogger), dim(0) {}

    Set_Impl::~Set_Impl(){
        for (auto &vec : container){
            delete vec;
        }
    }
    size_t Set_Impl::getIndex(IVector const *pSample, IVector::NORM norm, double tolerance) const {
        for (size_t i = 0; i < container.size(); ++i){
            bool isEqual;
            RESULT_CODE ans = IVector::equals(pSample, container[i], norm, tolerance, &isEqual, logger);
            if (ans == RESULT_CODE::SUCCESS) {
                if (isEqual) {
                    return i;
                }
            }
        }
        return -1;
    }

    RESULT_CODE Set_Impl::insert(const IVector* pVector, IVector::NORM norm, double tolerance) {
        if (pVector == nullptr){
            if (logger != nullptr){
                logger->log("In insert(...)", RESULT_CODE::BAD_REFERENCE);
            }
            return RESULT_CODE::BAD_REFERENCE;
        }
        if (!dim){
            dim = pVector->getDim();
            container.push_back(pVector->clone());
        } else{
            if (dim != pVector->getDim()){
                if (logger != nullptr){
                    logger->log("In insert(...)", RESULT_CODE::WRONG_DIM);
                }
                return RESULT_CODE::WRONG_DIM;
            } else{
                size_t ind = getIndex(pVector, norm, tolerance);
                if (ind == -1){
                    container.push_back(pVector->clone());
                }
            }
        }
        return RESULT_CODE::SUCCESS;
    }

    RESULT_CODE Set_Impl::get(IVector *&pVector, size_t index) const {
        if (index >= container.size()){
            if (logger != nullptr){
                logger->log("In get(...)", RESULT_CODE::OUT_OF_BOUNDS);
            }
            return RESULT_CODE::OUT_OF_BOUNDS;
        }
        pVector = container[index]->clone();
        return RESULT_CODE::SUCCESS;

    }

    RESULT_CODE Set_Impl::get(IVector *&pVector, IVector const *pSample, IVector::NORM norm, double tolerance) const {
        size_t index = getIndex(pSample, norm, tolerance);
        if (index){
            pSample = container[index]->clone();
            return RESULT_CODE::SUCCESS;
        }
        if (logger != nullptr){
            logger->log("", RESULT_CODE::NOT_FOUND);
        }
        return RESULT_CODE::NOT_FOUND;
    }

    size_t Set_Impl::getDim() const {
        return dim;
    }

    size_t Set_Impl::getSize() const {
        return container.size();
    }

    void Set_Impl::clear() {
        for (auto &vec : container){
            delete vec;
        }
        container.clear();
        dim = 0;
    }

    RESULT_CODE Set_Impl::erase(size_t index) {
        if (index >= container.size()){
            if (logger != nullptr){
                logger->log("In get(...)", RESULT_CODE::OUT_OF_BOUNDS);
            }
            return RESULT_CODE::OUT_OF_BOUNDS;
        }
        delete container[index];
        container.erase(container.begin() + index);
        return RESULT_CODE::SUCCESS;
    }

    RESULT_CODE Set_Impl::erase(IVector const *pSample, IVector::NORM norm, double tolerance) {
        size_t index = getIndex(pSample, norm, tolerance);
        if (index){
            delete container[index];
            container.erase(container.begin() + index);
            return RESULT_CODE::SUCCESS;
        }
        if (logger != nullptr){
            logger->log("", RESULT_CODE::NOT_FOUND);
        }
        return RESULT_CODE::NOT_FOUND;
    }

    ISet *Set_Impl::clone() const {
        auto * set = new Set_Impl(logger);
        set->dim = this->dim;
        for (auto &vec : container){
            set->container.push_back(vec->clone());
        }
        return set;
    }
}

ISet* ISet::createSet(ILogger* pLogger) {
    auto *newSet = new(std::nothrow) Set_Impl(pLogger);
    if (newSet == nullptr){
        if (pLogger != nullptr){
            pLogger->log("In createSet(...)", RESULT_CODE::OUT_OF_MEMORY);
        }
    }
    return newSet;
}
ISet* ISet::add(ISet const* pOperand1, ISet const* pOperand2, IVector::NORM norm, double tolerance, ILogger* pLogger){
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

    const auto *pOp2 = dynamic_cast<const Set_Impl*>(pOperand2);
    const auto *pOp1 = dynamic_cast<const Set_Impl*>(pOperand1);

    auto * newSet = new Set_Impl(pLogger);
    newSet->dim = pOp1->dim;
    for (auto &vec : pOp1->container){
        newSet->container.push_back(vec->clone());
    }

    for (auto &vec : pOp2->container){
        if (newSet->getIndex(vec, norm, tolerance) == -1){
            newSet->container.push_back(vec->clone());
        }
    }
    return newSet;
}

ISet* ISet::intersect(ISet const *pOperand1, ISet const *pOperand2, IVector::NORM norm, double tolerance,
                      ILogger *pLogger) {
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

    const auto *pOp2 = dynamic_cast<const Set_Impl*>(pOperand2);
    const auto *pOp1 = dynamic_cast<const Set_Impl*>(pOperand1);

    size_t lessSize = pOp1->container.size() < pOp2->container.size()  ? pOp1->container.size()  : pOp2->container.size() ;
    auto * newSet = new Set_Impl(pLogger);
    newSet->dim = pOp1->dim;

    if (pOp1->container.size() < pOp2->container.size()){
        for (auto &vec : pOp1->container){
            if (pOp2->getIndex(vec, norm, tolerance) != -1){
                newSet->container.push_back(vec->clone());
            }
        }
    } else{
        for (auto &vec : pOp2->container){
            if (pOp1->getIndex(vec, norm, tolerance) != -1){
                newSet->container.push_back(vec->clone());
            }
        }
    }
    return newSet;
}


