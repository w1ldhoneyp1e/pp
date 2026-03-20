#include "../include/Simulation.h"

#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

struct Simulation::Actor
{
    std::string name;
    Money cash = 0;
    mutable std::mutex mutex;

    explicit Actor(std::string actorName) : name(std::move(actorName)) {}
};

struct Simulation::EconomyContext
{
    explicit EconomyContext(bool verboseFlag)
        : bank(INITIAL_CASH), verbose(verboseFlag)
    {
    }

    Bank bank;
    bool verbose = true;

    Actor homer{"Homer"};
    Actor marge{"Marge"};
    Actor bart{"Bart"};
    Actor lisa{"Lisa"};
    Actor apu{"Apu"};
    Actor burns{"Burns"};

    AccountId homerAccount = 0;
    AccountId margeAccount = 0;
    AccountId apuAccount = 0;
    AccountId burnsAccount = 0;
};

Simulation::Simulation(bool verbose)
    : m_ctx(std::make_unique<EconomyContext>(verbose))
{
    m_ctx->homerAccount = m_ctx->bank.OpenAccount();
    m_ctx->margeAccount = m_ctx->bank.OpenAccount();
    m_ctx->apuAccount = m_ctx->bank.OpenAccount();
    m_ctx->burnsAccount = m_ctx->bank.OpenAccount();

    m_ctx->bank.DepositMoney(m_ctx->burnsAccount, 5'000);
    m_ctx->bank.DepositMoney(m_ctx->homerAccount, 2'000);
    m_ctx->bank.DepositMoney(m_ctx->margeAccount, 500);
    m_ctx->bank.DepositMoney(m_ctx->apuAccount, 500);

    AddCash(m_ctx->homer, 700);
    AddCash(m_ctx->marge, 300);
    AddCash(m_ctx->bart, 200);
    AddCash(m_ctx->lisa, 200);
    AddCash(m_ctx->apu, 300);
    AddCash(m_ctx->burns, 300);
}

Simulation::~Simulation() = default;

void Simulation::Run(bool parallel, std::chrono::seconds duration,
                     const std::atomic<bool> &stopRequested)
{
    const auto deadline = std::chrono::steady_clock::now() + duration;
    if (parallel)
    {
        RunParallel(deadline, stopRequested);

        return;
    }

    RunSingleThread(deadline, stopRequested);
}

SimulationReport Simulation::BuildReport() const
{
    SimulationReport report;
    report.operationsCount = m_ctx->bank.GetOperationsCount();
    report.actorCash = SumActorCash();
    report.bankCash = m_ctx->bank.GetCash();
    report.accountMoney = SumAccounts();
    report.totalMoney = report.bankCash + report.accountMoney;
    report.actorCashMatchesBankCash = (report.actorCash == report.bankCash);
    report.totalMoneyInvariant = (report.totalMoney == INITIAL_CASH);

    return report;
}

void Simulation::RunSingleThread(
    const std::chrono::steady_clock::time_point &deadline,
    const std::atomic<bool> &stopRequested)
{
    std::mt19937_64 rng(42);
    while (!stopRequested.load(std::memory_order_relaxed) &&
           std::chrono::steady_clock::now() < deadline)
    {
        HomerStep(rng);
        MargeStep(rng);
        BartStep(rng);
        LisaStep(rng);
        ApuStep(rng);
        BurnsStep(rng);
    }
}

void Simulation::RunParallel(
    const std::chrono::steady_clock::time_point &deadline,
    const std::atomic<bool> &stopRequested)
{
    auto runActor = [&](auto step, std::size_t seed)
    {
        std::mt19937_64 rng(seed);
        while (!stopRequested.load(std::memory_order_relaxed) &&
               std::chrono::steady_clock::now() < deadline)
        {
            step(rng);
            // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    std::thread homerThread(
        runActor, [&](std::mt19937_64 &rng) { HomerStep(rng); }, 1);
    std::thread margeThread(
        runActor, [&](std::mt19937_64 &rng) { MargeStep(rng); }, 2);
    std::thread bartThread(
        runActor, [&](std::mt19937_64 &rng) { BartStep(rng); }, 3);
    std::thread lisaThread(
        runActor, [&](std::mt19937_64 &rng) { LisaStep(rng); }, 4);
    std::thread apuThread(
        runActor, [&](std::mt19937_64 &rng) { ApuStep(rng); }, 5);
    std::thread burnsThread(
        runActor, [&](std::mt19937_64 &rng) { BurnsStep(rng); }, 6);

    homerThread.join();
    margeThread.join();
    bartThread.join();
    lisaThread.join();
    apuThread.join();
    burnsThread.join();
}

Money Simulation::GetCash(const Actor &actor)
{
    std::lock_guard<std::mutex> lock(actor.mutex);

    return actor.cash;
}

void Simulation::AddCash(Actor &actor, Money amount)
{
    std::lock_guard<std::mutex> lock(actor.mutex);
    actor.cash += amount;
}

bool Simulation::TryTransferCash(Actor &src, Actor &dst, Money amount)
{
    if (amount < 0)
    {
        return false;
    }

    std::scoped_lock lock(src.mutex, dst.mutex);
    if (src.cash < amount)
    {
        return false;
    }

    src.cash -= amount;
    dst.cash += amount;

    return true;
}

void Simulation::Print(const std::string &message) const
{
    if (!m_ctx->verbose)
    {
        return;
    }

    std::cout << message << '\n';
}

void Simulation::HomerStep(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<Money> familyDist(5, 15);
    std::uniform_int_distribution<Money> utilityDist(3, 10);
    std::uniform_int_distribution<Money> kidDist(1, 6);

    const Money familyAmount = familyDist(rng);
    if (m_ctx->bank.TrySendMoney(m_ctx->homerAccount, m_ctx->margeAccount,
                                 familyAmount))
    {
        Print("Homer -> Marge transfer: " + std::to_string(familyAmount));
    }

    const Money utilityAmount = utilityDist(rng);
    if (m_ctx->bank.TrySendMoney(m_ctx->homerAccount, m_ctx->burnsAccount,
                                 utilityAmount))
    {
        Print("Homer pays electricity: " + std::to_string(utilityAmount));
    }

    const Money bartCash = kidDist(rng);
    if (m_ctx->bank.TryWithdrawMoney(m_ctx->homerAccount, bartCash))
    {
        AddCash(m_ctx->bart, bartCash);
        Print("Homer gives Bart cash: " + std::to_string(bartCash));
    }

    const Money lisaCash = kidDist(rng);
    if (m_ctx->bank.TryWithdrawMoney(m_ctx->homerAccount, lisaCash))
    {
        AddCash(m_ctx->lisa, lisaCash);
        Print("Homer gives Lisa cash: " + std::to_string(lisaCash));
    }
}

void Simulation::MargeStep(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<Money> groceryDist(3, 12);
    const Money amount = groceryDist(rng);
    if (m_ctx->bank.TrySendMoney(m_ctx->margeAccount, m_ctx->apuAccount,
                                 amount))
    {
        Print("Marge buys groceries: " + std::to_string(amount));
    }
}

void Simulation::BartStep(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<Money> spendDist(1, 4);
    const Money amount = spendDist(rng);
    if (TryTransferCash(m_ctx->bart, m_ctx->apu, amount))
    {
        Print("Bart buys snacks for cash: " + std::to_string(amount));
    }
}

void Simulation::LisaStep(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<Money> spendDist(1, 4);
    const Money amount = spendDist(rng);
    if (TryTransferCash(m_ctx->lisa, m_ctx->apu, amount))
    {
        Print("Lisa buys books for cash: " + std::to_string(amount));
    }
}

void Simulation::ApuStep(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<Money> utilityDist(2, 8);
    std::uniform_int_distribution<Money> depositDist(1, 10);

    const Money utilityAmount = utilityDist(rng);
    if (m_ctx->bank.TrySendMoney(m_ctx->apuAccount, m_ctx->burnsAccount,
                                 utilityAmount))
    {
        Print("Apu pays electricity: " + std::to_string(utilityAmount));
    }

    const Money depositAmount = depositDist(rng);
    {
        std::lock_guard<std::mutex> apuLock(m_ctx->apu.mutex);
        if (m_ctx->apu.cash >= depositAmount)
        {
            try
            {
                m_ctx->bank.DepositMoney(m_ctx->apuAccount, depositAmount);
                m_ctx->apu.cash -= depositAmount;
                Print("Apu deposits cash: " + std::to_string(depositAmount));
            }
            catch (const BankOperationError &)
            {
            }
        }
    }
}

void Simulation::BurnsStep(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<Money> salaryDist(10, 20);
    const Money salary = salaryDist(rng);
    if (m_ctx->bank.TrySendMoney(m_ctx->burnsAccount, m_ctx->homerAccount,
                                 salary))
    {
        Print("Burns pays salary: " + std::to_string(salary));
    }
}

Money Simulation::SumActorCash() const
{
    return GetCash(m_ctx->homer) + GetCash(m_ctx->marge) +
           GetCash(m_ctx->bart) + GetCash(m_ctx->lisa) + GetCash(m_ctx->apu) +
           GetCash(m_ctx->burns);
}

Money Simulation::SumAccounts() const
{
    return m_ctx->bank.GetAccountBalance(m_ctx->homerAccount) +
           m_ctx->bank.GetAccountBalance(m_ctx->margeAccount) +
           m_ctx->bank.GetAccountBalance(m_ctx->apuAccount) +
           m_ctx->bank.GetAccountBalance(m_ctx->burnsAccount);
}
