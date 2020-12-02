//
// Created by Raghavasimhan Sankaranarayanan on 6/20/20.
//

#ifndef GOOGLEDRUMMINGARM_ERRORDEF_H
#define GOOGLEDRUMMINGARM_ERRORDEF_H

enum Error_t {
    kNoError = 0,
    kNotInitializedError,
    kNotImplementedError,
    kIllegalArgsError,

    kFileExtensionError,
    kFilePathNullError,
    kFileOpenError,
    kFileCloseError,
    kFileReadError,
    kFileWriteError,
    kFileEOFError,

    kSetValueError,
    kGetValueError,
    kNullError,

    kMemError,
    kNotEnoughMemoryError,
    kNotEnoughSamplesError,

    kBufferEmptyError,
    kBufferFullError,

    kUnknownError
};
#endif //GOOGLEDRUMMINGARM_ERRORDEF_H
