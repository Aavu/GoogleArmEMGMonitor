#ifndef ERRORDEF_H
#define ERRORDEF_H

enum Error_t {
    kNoError = 0,
    kNotInitializedError,
    kNotImplementedError,

    kFileExtensionError,
    kFilePathNullError,
    kFileOpenError,
    kFileCloseError,
    kFileReadError,
    kFileWriteError,
    kFileEOFError,

    kSetValueError,
    kGetValueError,

    kMemError,
    kNotEnoughMemoryError,
    kNotEnoughSamplesError,

    kBufferEmptyError,
    kBufferFullError,

    kUnknownError
};

#endif // ERRORDEF_H
