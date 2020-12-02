//
// Created by Raghavasimhan Sankaranarayanan on 6/20/20.
//

#include "Logger.h"

void Logger::init(Level level) {
    spdlog::set_pattern("%^[%r]\t[%s]\t[line %#]\t[---%l---]\t%v%$");
    spdlog::set_level((spdlog::level::level_enum)level);
    LOG_TRACE("Logger initialized");
}

//template<typename T>
//void Logger::pprintArray(T *array, int size)
