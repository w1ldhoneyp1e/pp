#include "../include/Bank.h"

#include <exception>
#include <iostream>
#include <string>
#include <thread>

namespace
{

void Assert(bool condition, const std::string &message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

template <typename TException, typename TCallable>
void AssertThrows(const std::string &message, TCallable &&callable)
{
    bool thrown = false;
    try
    {
        callable();
    }
    catch (const TException &)
    {
        thrown = true;
    }

    Assert(thrown, message);
}

void TestInitialCashValidation()
{
    AssertThrows<BankOperationError>("Negative initial cash must throw",
                                     [] { Bank bank(-1); });
}

void TestOpenCloseAccount()
{
    Bank bank(100);
    const auto account = bank.OpenAccount();
    Assert(bank.GetAccountBalance(account) == 0,
           "Opened account must have zero balance");
    bank.DepositMoney(account, 40);
    const Money closedMoney = bank.CloseAccount(account);
    Assert(closedMoney == 40, "CloseAccount must return account balance");
    Assert(bank.GetCash() == 100,
           "Cash must return to initial amount after close");
    AssertThrows<BankOperationError>("Closed account should be invalid",
                                     [&] { bank.GetAccountBalance(account); });
}

void TestDepositWithdraw()
{
    Bank bank(100);
    const auto account = bank.OpenAccount();
    bank.DepositMoney(account, 60);
    Assert(bank.GetCash() == 40, "Deposit must decrease cash");
    Assert(bank.GetAccountBalance(account) == 60,
           "Deposit must increase account balance");

    bank.WithdrawMoney(account, 15);
    Assert(bank.GetCash() == 55, "Withdraw must increase cash");
    Assert(bank.GetAccountBalance(account) == 45,
           "Withdraw must decrease account balance");
}

void TestSendAndTrySend()
{
    Bank bank(200);
    const auto a = bank.OpenAccount();
    const auto b = bank.OpenAccount();
    bank.DepositMoney(a, 100);
    Assert(bank.TrySendMoney(a, b, 70),
           "TrySendMoney should succeed on enough balance");
    Assert(bank.GetAccountBalance(a) == 30, "Source after transfer");
    Assert(bank.GetAccountBalance(b) == 70, "Destination after transfer");
    Assert(!bank.TrySendMoney(a, b, 100),
           "TrySendMoney should fail on insufficient funds");
    AssertThrows<BankOperationError>(
        "SendMoney should throw on insufficient funds",
        [&] { bank.SendMoney(a, b, 100); });
}

void TestValidationErrors()
{
    Bank bank(10);
    const auto account = bank.OpenAccount();
    AssertThrows<std::out_of_range>("Negative deposit must throw",
                                    [&] { bank.DepositMoney(account, -1); });
    AssertThrows<std::out_of_range>("Negative withdraw must throw",
                                    [&] { bank.WithdrawMoney(account, -1); });
    AssertThrows<std::out_of_range>("Negative transfer must throw", [&]
                                    { bank.SendMoney(account, account, -1); });
    AssertThrows<BankOperationError>("Unknown account must throw",
                                     [&] { bank.GetAccountBalance(9999); });
    AssertThrows<BankOperationError>("Deposit more than cash must throw",
                                     [&] { bank.DepositMoney(account, 11); });
}

void TestOperationsCount()
{
    Bank bank(100);
    const auto start = bank.GetOperationsCount();
    const auto account = bank.OpenAccount();
    const Money cashBefore = bank.GetCash();
    const Money balanceBefore = bank.GetAccountBalance(account);
    bank.DepositMoney(account, 20);
    const bool withdrew = bank.TryWithdrawMoney(account, 5);
    const auto end = bank.GetOperationsCount();

    Assert(cashBefore == 100, "Initial cash must be unchanged before deposit");
    Assert(balanceBefore == 0, "Initial balance must be zero");
    Assert(withdrew, "TryWithdrawMoney should succeed");
    Assert(end - start == 5,
           "Operations count must include read/write methods except getter");
}

void TestConcurrentTransfersPreserveMoney()
{
    Bank bank(1'000);
    const auto a = bank.OpenAccount();
    const auto b = bank.OpenAccount();
    bank.DepositMoney(a, 500);
    bank.DepositMoney(b, 500);

    auto worker = [&](int direction)
    {
        for (int i = 0; i < 50'000; ++i)
        {
            if (direction == 0)
            {
                const bool ok = bank.TrySendMoney(a, b, 1);
                (void)ok;
            }
            else
            {
                const bool ok = bank.TrySendMoney(b, a, 1);
                (void)ok;
            }
        }
    };

    std::thread t1(worker, 0);
    std::thread t2(worker, 1);
    std::thread t3(worker, 0);
    std::thread t4(worker, 1);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    const Money total =
        bank.GetCash() + bank.GetAccountBalance(a) + bank.GetAccountBalance(b);
    Assert(total == 1'000, "Concurrent transfers must preserve total money");
}

} // namespace

int main()
{
    try
    {
        TestInitialCashValidation();
        TestOpenCloseAccount();
        TestDepositWithdraw();
        TestSendAndTrySend();
        TestValidationErrors();
        TestOperationsCount();
        TestConcurrentTransfersPreserveMoney();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Test failed: " << ex.what() << '\n';

        return 1;
    }

    std::cout << "All tests passed\n";

    return 0;
}
