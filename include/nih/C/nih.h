/* This file is part of NIH.
 *
 * Copyright (c) 2019 Jiaming Yuan <jm.yuan@outlook.com>
 *
 * NIH is free software: you can redistribute it and/or modify it under the
 * terms of the Lesser GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * NIH is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Lesser GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with NIH.  If not, see <https://www.gnu.org/licenses/>.
 */
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