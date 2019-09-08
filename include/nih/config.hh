#ifndef _NIH_CONFIG_HH_
#define _NIH_CONFIG_HH_

namespace nih {
#if defined(__GNUC__)
#define NIH_EXPECT(cond, ret)  __builtin_expect((cond), (ret))
#else
#define NIH_EXPECT(cond, ret) (cond)
#endif  // defined(__GNUC__)
}

#endif  // _NIH_CONFIG_HH_