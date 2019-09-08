#include <gtest/gtest.h>
#include <nih/path.hh>

namespace nih {
TEST(Path, Join) {
  {
    Path foo {"foo"};
    Path bar {"bar"};
    Path joined = foo + bar;
    ASSERT_EQ(joined.str(), "foo/bar");
  }

  {
    Path foo {"foo/"};
    Path bar {"bar"};
    Path joined = foo + bar;
    ASSERT_EQ(joined.str(), "foo/bar");
  }

  {
    Path foo {"foo/"};
    Path bar {"/bar"};
    Path joined = foo + bar;
    ASSERT_EQ(joined.str(), "foo/bar");
  }

  {
    Path foo {"foo/"};
    Path bar {"/bar"};
    Path joined = Path::join(foo, bar);
    ASSERT_EQ(joined.str(), "foo/bar");
  }

  {
    Path foo{"foo"};
    Path joined = Path::join(foo);
    ASSERT_EQ(joined.str(), "foo");
  }

  {
    Path foo;
    Path joined = Path::join(foo);
    ASSERT_EQ(joined.str(), "");
  }
}

TEST(Path, Curdir) {
  auto curdir = Path::curdir();
  ASSERT_NE(curdir.str().size(), 0);
}

TEST(Path, Test) {
  auto curdir = Path::curdir();
  ASSERT_TRUE(curdir.isDir());

  auto file = Path{__FILE__};
  ASSERT_TRUE(file.isFile());

  ASSERT_FALSE(curdir.isSymlink());
  ASSERT_FALSE(file.isSymlink());
}

}  // namespace nih