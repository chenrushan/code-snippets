// compile with -lgtest
// 
// http://www.ibm.com/developerworks/aix/library/au-googletestingframework.html
// 提供一个不错的 tutorial
#include <gtest/gtest.h>

// ======================================================================
// TEST() 一般用于最简单的 test，不需要什么 context setup
// ======================================================================

int abs(int i1)
{
    return i1 < 0 ? -i1 : i1;
}

TEST(AbsTest, PositiveNo)
{
    EXPECT_EQ(5, abs(5));
}

TEST(AbsTest, NegativeNo)
{
    EXPECT_EQ(5, abs(-5));
}

// ======================================================================
// TEST_F() 用于需要 setup context 的情况，比如需要测试一个类，不希望每次
// 都在 TEST() 中创建一个，这时候就用 TEST_F()
// ======================================================================

// class to be tested
struct BankAccount {
    int balance = 0;

    void deposite(int num) {
        balance += num;
    }

    bool withdraw(int num) {
        if (num <= balance) {
            balance -= num;
        } else {
            return false;
        }
        return true;
    }
};

struct BankAccountTest : testing::Test {
    BankAccount account;
};

// TEST_F 会自动生成为 BankAccountTest 的 member function
TEST_F(BankAccountTest, BankAccountStartsEmpty)
{
    EXPECT_EQ(0, account.balance);
}

TEST_F(BankAccountTest, CanDepositeMoney)
{
    account.deposite(100);
    EXPECT_EQ(100, account.balance);
}

// ======================================================================
// TEST_P() 当你需要参数化的 test 时，就用 TEST_P()，比如下面用
// 这种方法来测试 withdraw 功能
// ======================================================================

struct AccountState {
    int initBalance;
    int withdrawAmount;
    int finalBalance;
    bool success;
};

struct WithdrawAccountTest : BankAccountTest, testing::WithParamInterface<AccountState> {
    WithdrawAccountTest() {
        account.balance = GetParam().initBalance;
    }
};

// TEST_P 会自动生成为 BankAccountTest 的 member function
TEST_P(WithdrawAccountTest, FinalBalance)
{
    auto as = GetParam();
    auto success = account.withdraw(as.withdrawAmount);
    EXPECT_EQ(as.finalBalance, account.balance);
    EXPECT_EQ(as.success, success);
}

// 使用 TEST_P() 的话，还需要额外提供一个 INSTANTIATE_TEST_CASE_P() 来
// 开始真正的 test，这个函数提供 test 需要的配置
INSTANTIATE_TEST_CASE_P(Default, WithdrawAccountTest,
    testing::Values(
        AccountState{100, 50, 50, true},
        AccountState{100, 200, 100, false}
    ));

// ======================================================================
// main
// ======================================================================

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

