import multiprocessing
import pickle
import subprocess
import pathlib
import matplotlib
import pandas as pd
import numpy as np
import itertools
import collections
import argparse

sst_path = pathlib.Path(__file__).parent.joinpath("sst")
dam_path = pathlib.Path(__file__).parent.joinpath("dam")


def setup_sst():
    make_job = subprocess.run(
        ["make"], check=True, cwd=sst_path.joinpath("src"), capture_output=True
    )
    if make_job.stdout:
        print("Make Job:")
        print(make_job.stdout.decode())

    if make_job.stderr:
        print("Make Err:")
        print(make_job.stderr.decode())


def setup_dam():
    subprocess.run(["cargo", "update"], cwd=dam_path, check=True)
    dam_job = subprocess.run(
        ["cargo", "build", "--profile", "release"],
        check=True,
        cwd=dam_path,
        capture_output=True,
    )

    if dam_job.stdout:
        print("DAM Job:")
        print(dam_job.stdout.decode())

    if dam_job.stderr:
        print("DAM Err:")
        print(dam_job.stderr.decode())


def run_sst(
    threads: int,
    fib_size: int,
    iters: int,
    depth: int,
    trees: int,
    latency: int,
    chan_depth: int,
    imbalance: int,
):
    cmd = [
        "/usr/bin/time",
        "-f",
        "%e",
        "sst",
        "benchmark.py",
        f"-n {threads}",
        "--",
        f"--fib-size={fib_size}",
        f"--iters={iters}",
        f"--depth={depth}",
        f"--num-trees={trees}",
        f"--latency={latency}ns",
        f"--channel-depth={chan_depth}",
        f"--imbalance={imbalance}",
    ]
    job = subprocess.run(
        cmd, capture_output=True, cwd=sst_path.joinpath("tests"), check=True
    )
    return job


def run_dam(
    fib_size: int,
    iters: int,
    depth: int,
    trees: int,
    latency: int,
    chan_depth: int | None,
    imbalance: int,
    fifo: bool,
    opt: bool,
):
    cmd = [
        "/usr/bin/time",
        "-f",
        "%e",
        "target/release/dam-compare",
        f"--fib-size={fib_size}",
        f"--iters={iters}",
        f"--depth={depth}",
        f"--num-trees={trees}",
        f"--latency={latency}",
        f"--imbalance={imbalance}",
    ]
    if fifo:
        cmd.append("--fifo-mode")
    if opt:
        cmd.append("--opt")
    if chan_depth:
        cmd.append(f"--channel-depth={chan_depth}")
    job = subprocess.run(cmd, capture_output=True, cwd=dam_path, check=True)
    return job


def parse_output(job: subprocess.CompletedProcess):
    seconds = float(job.stderr.decode())
    return seconds


# Sweep space
num_iters = [100000]
fib_size = [16, 20]

# step in chunks of 4
num_trees = [2, 8, 32]
imbalance = [0, 4]
depths = [8, 10]

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-r", "--repeats", action="store", type=int, default=1, dest="repeats"
    )
    parser.add_argument(
        "-c", "--chan-depth", action="store", type=int, default=4096, dest="chan_depth"
    )
    args = parser.parse_args()

    setup_sst()
    setup_dam()

    sst_results = collections.defaultdict(list)

    dam_opts = [True, False]

    dam_results = {opt: collections.defaultdict(list) for opt in dam_opts}

    repeats = args.repeats
    chan_depth = args.chan_depth

    for iters, fib, trees, imba, depth in itertools.product(
        num_iters, fib_size, num_trees, imbalance, depths
    ):

        print("Starting Iter:", iters, fib, trees, imba, depth)
        for _ in range(repeats):
            sst_result = parse_output(
                run_sst(
                    multiprocessing.cpu_count(),
                    fib,
                    iters,
                    depth,
                    trees,
                    1,
                    chan_depth,
                    imba,
                )
            )
            print("SST", sst_result)
            sst_results[(iters, fib, trees, imba, depth)].append(sst_result)

        with open("sst_results.pkl", "wb") as pkl:
            pickle.dump(sst_results, pkl)

        for use_fifo in dam_opts:
            for _ in range(repeats):
                dam_result = parse_output(
                    run_dam(
                        fib,
                        iters,
                        depth,
                        trees,
                        1,
                        chan_depth,
                        imba,
                        use_fifo,
                        False,
                    )
                )
                print("DAM", use_fifo, dam_result)
                dam_results[use_fifo][(iters, fib, trees, imba, depth)].append(
                    dam_result
                )

            with open("dam_results.pkl", "wb") as pkl:
                pickle.dump(dam_results, pkl)
