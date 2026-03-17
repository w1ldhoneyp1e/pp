#include "../include/Bank.h"

Bank::Bank(Money cash) : cash_(cash)
{
    if (cash < 0)
    {
        throw BankOperationError("Initial cash cannot be negative");
    }
}

unsigned long long Bank::GetOperationsCount() const
{
    return operationsCount_.load(std::memory_order_relaxed);
}

void Bank::SendMoney(AccountId srcAccountId, AccountId dstAccountId,
                     Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto [srcAccount, dstAccount] =
        GetTwoAccountsOrThrow(srcAccountId, dstAccountId);
    if (srcAccountId == dstAccountId)
    {
        std::lock_guard<std::mutex> lock(srcAccount->mutex);
        if (srcAccount->balance < amount)
        {
            throw BankOperationError("Insufficient funds on source account");
        }

        return;
    }

    std::scoped_lock lock(srcAccount->mutex, dstAccount->mutex);
    if (srcAccount->balance < amount)
    {
        throw BankOperationError("Insufficient funds on source account");
    }

    srcAccount->balance -= amount;
    dstAccount->balance += amount;
}

bool Bank::TrySendMoney(AccountId srcAccountId, AccountId dstAccountId,
                        Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto [srcAccount, dstAccount] =
        GetTwoAccountsOrThrow(srcAccountId, dstAccountId);
    if (srcAccountId == dstAccountId)
    {
        std::lock_guard<std::mutex> lock(srcAccount->mutex);
        if (srcAccount->balance < amount)
        {
            return false;
        }

        return true;
    }

    std::scoped_lock lock(srcAccount->mutex, dstAccount->mutex);
    if (srcAccount->balance < amount)
    {
        return false;
    }

    srcAccount->balance -= amount;
    dstAccount->balance += amount;

    return true;
}

Money Bank::GetCash() const
{
    Tick();
    std::lock_guard<std::mutex> lock(cashMutex_);

    return cash_;
}

Money Bank::GetAccountBalance(AccountId accountId) const
{
    Tick();
    auto account = GetAccountOrThrow(accountId);
    std::lock_guard<std::mutex> lock(account->mutex);

    return account->balance;
}

void Bank::WithdrawMoney(AccountId account, Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto accountPtr = GetAccountOrThrow(account);
    std::scoped_lock lock(accountPtr->mutex, cashMutex_);
    if (accountPtr->balance < amount)
    {
        throw BankOperationError("Insufficient funds on account");
    }

    accountPtr->balance -= amount;
    cash_ += amount;
}

bool Bank::TryWithdrawMoney(AccountId account, Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto accountPtr = GetAccountOrThrow(account);
    std::scoped_lock lock(accountPtr->mutex, cashMutex_);
    if (accountPtr->balance < amount)
    {
        return false;
    }

    accountPtr->balance -= amount;
    cash_ += amount;

    return true;
}

void Bank::DepositMoney(AccountId account, Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto accountPtr = GetAccountOrThrow(account);
    std::scoped_lock lock(accountPtr->mutex, cashMutex_);
    if (cash_ < amount)
    {
        throw BankOperationError("Insufficient cash in circulation");
    }

    cash_ -= amount;
    accountPtr->balance += amount;
}

AccountId Bank::OpenAccount()
{
    Tick();
    std::unique_lock<std::shared_mutex> lock(accountsMutex_);

    const AccountId accountId = nextAccountId_;
    ++nextAccountId_;
    accounts_.emplace(accountId, std::make_shared<Account>());

    return accountId;
}

Money Bank::CloseAccount(AccountId accountId)
{
    Tick();
    std::unique_lock<std::shared_mutex> lock(accountsMutex_);
    auto it = accounts_.find(accountId);
    if (it == accounts_.end())
    {
        throw BankOperationError("Account does not exist");
    }

    auto account = it->second;
    std::lock_guard<std::mutex> accountLock(account->mutex);
    const Money balance = account->balance;
    account->balance = 0;

    {
        std::lock_guard<std::mutex> cashLock(cashMutex_);
        cash_ += balance;
    }

    accounts_.erase(it);

    return balance;
}

void Bank::Tick() const
{
    operationsCount_.fetch_add(1, std::memory_order_relaxed);
}

void Bank::ValidateNonNegative(Money amount)
{
    if (amount < 0)
    {
        throw std::out_of_range("Amount cannot be negative");
    }
}

Bank::AccountPtr Bank::GetAccountOrThrow(AccountId accountId) const
{
    std::shared_lock<std::shared_mutex> lock(accountsMutex_);
    auto it = accounts_.find(accountId);
    if (it == accounts_.end())
    {
        throw BankOperationError("Account does not exist");
    }

    return it->second;
}

std::pair<Bank::AccountPtr, Bank::AccountPtr>
Bank::GetTwoAccountsOrThrow(AccountId srcAccountId,
                            AccountId dstAccountId) const
{
    std::shared_lock<std::shared_mutex> lock(accountsMutex_);
    auto srcIt = accounts_.find(srcAccountId);
    auto dstIt = accounts_.find(dstAccountId);
    if (srcIt == accounts_.end() || dstIt == accounts_.end())
    {
        throw BankOperationError(
            "Source or destination account does not exist");
    }

    return {srcIt->second, dstIt->second};
}
