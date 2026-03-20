#pragma once

#include "Bank.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <random>
#include <string>

struct SimulationReport
{
    unsigned long long operationsCount = 0;
    Money actorCash = 0;
    Money bankCash = 0;
    Money accountMoney = 0;
    Money totalMoney = 0;
    bool actorCashMatchesBankCash = false;
    bool totalMoneyInvariant = false;
};

class Simulation
{
  public:
    static constexpr Money INITIAL_CASH = 10'000;

    explicit Simulation(bool verbose);
    ~Simulation();

    void Run(bool parallel, std::chrono::seconds duration,
             const std::atomic<bool> &stopRequested);
    SimulationReport BuildReport() const;

  private:
    struct Actor;
    struct EconomyContext;

    void RunSingleThread(const std::chrono::steady_clock::time_point &deadline,
                         const std::atomic<bool> &stopRequested);
    void RunParallel(const std::chrono::steady_clock::time_point &deadline,
                     const std::atomic<bool> &stopRequested);

    static Money GetCash(const Actor &actor);
    static void AddCash(Actor &actor, Money amount);
    static bool TryTransferCash(Actor &src, Actor &dst, Money amount);

    void Print(const std::string &message) const;

    void HomerStep(std::mt19937_64 &rng);
    void MargeStep(std::mt19937_64 &rng);
    void BartStep(std::mt19937_64 &rng);
    void LisaStep(std::mt19937_64 &rng);
    void ApuStep(std::mt19937_64 &rng);
    void BurnsStep(std::mt19937_64 &rng);

    Money SumActorCash() const;
    Money SumAccounts() const;

  private:
    std::unique_ptr<EconomyContext> m_ctx;
};
