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
#ifndef C_LOGGING_H_
#define C_LOGGING_H_

#include "nih/C/nih.h"

NIH_API NIH_ErrCode nih_log(char const* msg, int verbosity);
NIH_API NIH_ErrCode nih_logDefault(char const* msg);

NIH_API NIH_ErrCode nih_setLogVerbosity(char const* msg);

#endif  // C_LOGGING_H_