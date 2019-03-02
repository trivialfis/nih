#ifndef NIH_C_H_
#define NIH_C_H_

#ifdef __cplusplus
#define __NIH_EXTERN_C extern "C"
#else
#define __NIH_EXTERN_C
#endif  // __cplusplus

#if defined(_MSC_VER) || defined(_WIN32) || defined (__CYGWIN__)
#error "Windows is not supported."
#elif defined(__GNUC__)

#define NIH_API      __NIH_EXTERN_C __attribute__ ((visibility ("default")))
#define NIH_Internal __NIH_EXTERN_C __attribute__ ((visibility ("hidden")))

#else

#define NIH_API      __NIH_EXTERN_C
#define NIH_Internal __NIH_EXTERN_C

#endif  // Platforms

enum __NIH_ErrorCode {
  kSuccess = 0,
  kWarning = 1,
  kFatal = 2
};
typedef int NIH_ErrCode;

#endif  // NIH_C_H_