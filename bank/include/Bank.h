#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <unordered_map>

using AccountId = unsigned long long;
using Money = long long;

class BankOperationError : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class Bank
{
public:
    explicit Bank(Money cash);

    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;

    unsigned long long GetOperationsCount() const;

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount);
    [[nodiscard]] bool TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount);

    [[nodiscard]] Money GetCash() const;
    Money GetAccountBalance(AccountId accountId) const;

    void WithdrawMoney(AccountId account, Money amount);
    [[nodiscard]] bool TryWithdrawMoney(AccountId account, Money amount);

    void DepositMoney(AccountId account, Money amount);

    [[nodiscard]] AccountId OpenAccount();
    [[nodiscard]] Money CloseAccount(AccountId accountId);

private:
    struct Account
    {
        Money balance = 0;
        mutable std::mutex mutex;
    };

    using AccountPtr = std::shared_ptr<Account>;

    void Tick() const;
    static void ValidateNonNegative(Money amount);

    AccountPtr GetAccountOrThrow(AccountId accountId) const;
    std::pair<AccountPtr, AccountPtr> GetTwoAccountsOrThrow(AccountId srcAccountId, AccountId dstAccountId) const;

private:
    mutable std::shared_mutex accountsMutex_;
    std::unordered_map<AccountId, AccountPtr> accounts_;
    AccountId nextAccountId_ = 1;

    mutable std::mutex cashMutex_;
    Money cash_ = 0;

    mutable std::atomic<unsigned long long> operationsCount_ = 0;
};
