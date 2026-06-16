#!/usr/bin/env python3

import csv
import pathlib
import sys

import matplotlib.pyplot as plt


def ReadRows(path: pathlib.Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as file:
        return list(csv.DictReader(file))


def GetParallelRows(rows: list[dict[str, str]]) -> list[dict[str, str]]:
    return [row for row in rows if row["implementation"] != "single_thread"]


def PlotTime(rows: list[dict[str, str]], output_path: pathlib.Path) -> None:
    baseline_row = next(
        row for row in rows if row["implementation"] == "single_thread"
    )
    baseline = float(baseline_row["median_ms"])
    parallel_rows = GetParallelRows(rows)
    thread_counts = sorted({int(row["threads"]) for row in parallel_rows})

    plt.figure(figsize=(10, 6))
    plt.plot(
        thread_counts,
        [baseline] * len(thread_counts),
        marker="o",
        linestyle="--",
        label="single_thread",
    )

    implementations = sorted({row["implementation"] for row in parallel_rows})
    for implementation in implementations:
        selected = sorted(
            (
                (int(row["threads"]), float(row["median_ms"]))
                for row in parallel_rows
                if row["implementation"] == implementation
            ),
            key=lambda item: item[0],
        )
        plt.plot(
            [item[0] for item in selected],
            [item[1] for item in selected],
            marker="o",
            label=implementation,
        )

    plt.xlabel("Number of threads")
    plt.ylabel("Median execution time, ms")
    plt.title("RGB histogram construction time")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=160)
    plt.close()


def PlotSpeedup(rows: list[dict[str, str]], output_path: pathlib.Path) -> None:
    parallel_rows = GetParallelRows(rows)
    implementations = sorted({row["implementation"] for row in parallel_rows})

    plt.figure(figsize=(10, 6))

    for implementation in implementations:
        selected = sorted(
            (
                (int(row["threads"]), float(row["speedup"]))
                for row in parallel_rows
                if row["implementation"] == implementation
            ),
            key=lambda item: item[0],
        )
        plt.plot(
            [item[0] for item in selected],
            [item[1] for item in selected],
            marker="o",
            label=implementation,
        )

    all_thread_counts = sorted({int(row["threads"]) for row in parallel_rows})
    plt.plot(
        all_thread_counts,
        all_thread_counts,
        linestyle="--",
        label="ideal speedup",
    )

    plt.xlabel("Number of threads")
    plt.ylabel("Speedup relative to single thread")
    plt.title("RGB histogram speedup")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=160)
    plt.close()


def Main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(
            "Usage: plot_benchmark.py RESULTS.csv OUTPUT_DIRECTORY"
        )

    input_path = pathlib.Path(sys.argv[1])
    output_directory = pathlib.Path(sys.argv[2])
    output_directory.mkdir(parents=True, exist_ok=True)

    rows = ReadRows(input_path)
    if not rows:
        raise SystemExit("The CSV file contains no benchmark rows")

    PlotTime(rows, output_directory / "time_vs_threads.png")
    PlotSpeedup(rows, output_directory / "speedup_vs_threads.png")

    print(output_directory / "time_vs_threads.png")
    print(output_directory / "speedup_vs_threads.png")


if __name__ == "__main__":
    Main()
