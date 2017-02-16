// compile with -lgtest
//
// http://www.ibm.com/developerworks/aix/library/au-googletestingframework.html
// 提供一个不错的 tutorial
#include <gtest/gtest.h>

TEST(Nothing, Nothing) { ASSERT_EQ(0, 0); }

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
