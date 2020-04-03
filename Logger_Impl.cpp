//
// Created by Dmitry Kozlov on 3/13/2020.
//
#include "interfaces/ILogger.h"
#include "interfaces/RC.h"
#include <set>
#include <cstdio>

ILogger::~ILogger() = default;

namespace {
    class Logger_Impl : ILogger{
    public:
        explicit Logger_Impl(void* pClient);
        void log(char const* pMsg, enum RESULT_CODE err) override;
        RESULT_CODE setLogFile(char const* pLogFile) override;
        void destroyLogger(void* pClient) override;
        friend ILogger* ILogger::createLogger(void *pClient);
    protected:
        static Logger_Impl * logger;
        static FILE * logFile;
        static std::set<void *> subscribers;
    };
    Logger_Impl * Logger_Impl::logger;
    FILE * Logger_Impl::logFile;
    std::set<void *> Logger_Impl::subscribers;
}

Logger_Impl::Logger_Impl(void *pClient) {
    subscribers.insert(pClient);
}

void Logger_Impl::log(char const *pMsg, enum RESULT_CODE err) {
    fprintf(logFile, "Error №%d: ", err);
    switch(err){
        case RESULT_CODE::WRONG_DIM:
            fprintf(logFile, "Wrong dimensions | ");
            break;
        case RESULT_CODE::NAN_VALUE:
            fprintf(logFile, "Some values are NAN | ");
            break;
        case RESULT_CODE::BAD_REFERENCE:
            fprintf(logFile, "Some operands are nullptr | ");
            break;
        case RESULT_CODE::WRONG_ARGUMENT:
            fprintf(logFile, "Unknown type of norm | ");
            break;
        case RESULT_CODE::OUT_OF_BOUNDS:
            fprintf(logFile, "Index is out of bounds | ");
            break;
        case RESULT_CODE::OUT_OF_MEMORY:
            fprintf(logFile, "Not enough memory | ");
            break;
        case RESULT_CODE::CALCULATION_ERROR:
            fprintf(logFile, "Result of operation is NAN | ");
            break;
        case RESULT_CODE::NOT_FOUND:
            fprintf(logFile, "Not found element | ");
            break;
        default:
            break;
    }
    fprintf(logFile, "%s", pMsg);
}

RESULT_CODE Logger_Impl::setLogFile(char const *pLogFile) {
    fclose(logFile);
    logFile = fopen(pLogFile, "w");
    if (logFile == nullptr){
        return RESULT_CODE::FILE_ERROR;
    }
    return RESULT_CODE::SUCCESS;
}


ILogger* ILogger::createLogger(void *pClient) {
    if (Logger_Impl::subscribers.empty()){
        Logger_Impl::logger = new Logger_Impl(pClient);
        Logger_Impl::logFile = fopen("log.txt", "w");
        if (Logger_Impl::logger == nullptr || Logger_Impl::logFile == nullptr){
            return nullptr;
        }
    } else{
        Logger_Impl::subscribers.insert(pClient);
    }
    return Logger_Impl::logger;
}

void Logger_Impl::destroyLogger(void *pClient) {
    auto it = subscribers.find(pClient);
    if (it != subscribers.end()){
        subscribers.erase(it);
        if (subscribers.empty()){
            delete logger;
            fclose(logFile);
        }
    }
}




