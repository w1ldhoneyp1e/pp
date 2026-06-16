#!/usr/bin/env python3

import csv
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt


def ReadResults(path: Path):
    results = defaultdict(
        lambda: {
            "threads": [],
            "times": [],
        }
    )

    with path.open("r", encoding="utf-8") as file:
        for line in file:
            line = line.strip()

            if not line:
                continue

            if line.startswith("Threads"):
                continue

            if set(line) == {"-"}:
                continue

            parts = line.split()

            if len(parts) != 3:
                continue

            threadCount = int(parts[0])
            implementation = parts[1]
            milliseconds = float(parts[2])

            results[implementation]["threads"].append(threadCount)
            results[implementation]["times"].append(milliseconds)

    return results


def Main():
    inputPath = (
        Path(sys.argv[1])
        if len(sys.argv) > 1
        else Path("results/benchmark.csv")
    )

    outputPath = (
        Path(sys.argv[2])
        if len(sys.argv) > 2
        else Path("results/thread_pool_benchmark.png")
    )

    results = ReadResults(inputPath)

    labels = {
        "lock_free": "Lock-free ThreadPool",
        "lock_based": "Lock-based ThreadPool",
        "boost_asio": "boost::asio::thread_pool",
    }

    for implementation, values in results.items():
        plt.plot(
            values["threads"],
            values["times"],
            marker="o",
            markersize=3,
            label=labels.get(implementation, implementation),
        )

    plt.xlabel("Количество потоков")
    plt.ylabel("Время выполнения, мс")
    plt.title("Зависимость времени выполнения задач от количества потоков")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()

    outputPath.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(outputPath, dpi=160)

    print(f"График сохранён в {outputPath}")


if __name__ == "__main__":
    Main()
