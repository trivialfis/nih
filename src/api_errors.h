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