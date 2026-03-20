#include "../include/Bank.h"

Bank::Bank(Money cash) : m_cash(cash)
{
    if (cash < 0)
    {
        throw BankOperationError("Initial cash cannot be negative");
    }
}

unsigned long long Bank::GetOperationsCount() const
{
    return m_operationsCount.load(std::memory_order_relaxed);
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
        std::lock_guard<std::shared_mutex> lock(srcAccount->mutex);
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
        std::lock_guard<std::shared_mutex> lock(srcAccount->mutex);
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
    std::lock_guard<std::mutex> lock(m_cashMutex);

    return m_cash;
}

Money Bank::GetAccountBalance(AccountId accountId) const
{
    Tick();
    auto account = GetAccountOrThrow(accountId);
    std::shared_lock<std::shared_mutex> lock(account->mutex);

    return account->balance;
}

void Bank::WithdrawMoney(AccountId account, Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto accountPtr = GetAccountOrThrow(account);
    std::scoped_lock lock(accountPtr->mutex, m_cashMutex);
    if (accountPtr->balance < amount)
    {
        throw BankOperationError("Insufficient funds on account");
    }

    accountPtr->balance -= amount;
    m_cash += amount;
}

bool Bank::TryWithdrawMoney(AccountId account, Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto accountPtr = GetAccountOrThrow(account);
    std::scoped_lock lock(accountPtr->mutex, m_cashMutex);
    if (accountPtr->balance < amount)
    {
        return false;
    }

    accountPtr->balance -= amount;
    m_cash += amount;

    return true;
}

void Bank::DepositMoney(AccountId account, Money amount)
{
    Tick();
    ValidateNonNegative(amount);

    auto accountPtr = GetAccountOrThrow(account);
    std::scoped_lock lock(accountPtr->mutex, m_cashMutex);
    if (m_cash < amount)
    {
        throw BankOperationError("Insufficient cash in circulation");
    }

    m_cash -= amount;
    accountPtr->balance += amount;
}

AccountId Bank::OpenAccount()
{
    Tick();
    std::lock_guard<std::shared_mutex> lock(m_accountsMutex);

    const AccountId accountId = m_nextAccountId;
    ++m_nextAccountId;
    m_accounts.emplace(accountId, std::make_shared<Account>());

    return accountId;
}

Money Bank::CloseAccount(AccountId accountId)
{
    Tick();
    std::lock_guard<std::shared_mutex> lock(m_accountsMutex);
    auto it = m_accounts.find(accountId);
    if (it == m_accounts.end())
    {
        throw BankOperationError("Account does not exist");
    }

    auto account = it->second;
    std::lock_guard<std::shared_mutex> accountLock(account->mutex);
    const Money balance = account->balance;
    account->balance = 0;

    {
        std::lock_guard<std::mutex> cashLock(m_cashMutex);
        m_cash += balance;
    }

    m_accounts.erase(it);

    return balance;
}

void Bank::Tick() const
{
    m_operationsCount.fetch_add(1, std::memory_order_relaxed);
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
    std::shared_lock<std::shared_mutex> lock(m_accountsMutex);
    auto it = m_accounts.find(accountId);
    if (it == m_accounts.end())
    {
        throw BankOperationError("Account does not exist");
    }

    return it->second;
}

std::pair<Bank::AccountPtr, Bank::AccountPtr>
Bank::GetTwoAccountsOrThrow(AccountId srcAccountId,
                            AccountId dstAccountId) const
{
    std::shared_lock<std::shared_mutex> lock(m_accountsMutex);
    auto srcIt = m_accounts.find(srcAccountId);
    auto dstIt = m_accounts.find(dstAccountId);
    if (srcIt == m_accounts.end() || dstIt == m_accounts.end())
    {
        throw BankOperationError(
            "Source or destination account does not exist");
    }

    return {srcIt->second, dstIt->second};
}
