#include "../include/Cli.h"
#include "../include/Simulation.h"

#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>

namespace
{
std::atomic<bool> gStopRequested = false;

void HandleSignal(int)
{
    gStopRequested.store(true, std::memory_order_relaxed);
}
} // namespace

int main(int argc, char **argv)
{
    try
    {
        std::signal(SIGINT, HandleSignal);
        std::signal(SIGTERM, HandleSignal);

        const CliOptions options = ParseArgs(argc, argv);
        Simulation simulation(options.verbose);
        simulation.Run(options.parallel, options.duration, gStopRequested);
        const SimulationReport report = simulation.BuildReport();

        std::cout << "Operations count: " << report.operationsCount << '\n';
        std::cout << "Actor cash: " << report.actorCash << '\n';
        std::cout << "Bank cash: " << report.bankCash << '\n';
        std::cout << "Account money: " << report.accountMoney << '\n';
        std::cout << "Total money: " << report.totalMoney << '\n';
        std::cout << "Actor cash == bank cash: "
                  << (report.actorCashMatchesBankCash ? "OK" : "FAIL") << '\n';
        std::cout << "Total money invariant: "
                  << (report.totalMoneyInvariant ? "OK" : "FAIL") << '\n';
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';

        return 1;
    }

    return 0;
}
