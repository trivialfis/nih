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
#ifndef API_ERRORS_H_
#define API_ERRORS_H_

#ifndef ISSUE_URL
#define ISSUE_URL() "https://github.com/trivialfis/nih"
#endif

#define C_API_BEG() try {

#define C_API_END()                                                     \
  } catch (nih::NIHError const& e) {                                    \
    std::cerr << e.what();                                              \
    return static_cast<NIH_ErrCode>(kFatal);                            \
  } catch (std::exception const& e) {                                   \
    LOG(FATAL) << "Internal Error. "  << e.what() << "\n"               \
               << "Please open a bug report in: " << ISSUE_URL();       \
  }                                                                     \
  return static_cast<NIH_ErrCode>(kSuccess);

#endif  // API_ERRORS_H_