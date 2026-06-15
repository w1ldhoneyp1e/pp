#!/usr/bin/env python3

import csv
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt


def ReadResults(path: Path):
    results = defaultdict(lambda: {"threads": [], "times": []})

    with path.open("r", encoding="utf-8", newline="") as file:
        reader = csv.DictReader(file)

        for row in reader:
            implementation = row["implementation"]
            results[implementation]["threads"].append(
                int(row["threads"]))
            results[implementation]["times"].append(
                float(row["time_ms"]))

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
