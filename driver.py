import multiprocessing
import subprocess
import pathlib
import matplotlib
import pandas as pd
import numpy as np
import itertools
import collections

sst_path = pathlib.Path(__file__).parent.joinpath("sst")
dam_path = pathlib.Path(__file__).parent.joinpath("dam")

print(sst_path)

def setup_sst():
    make_job = subprocess.run(["make"], check=True, cwd=sst_path.joinpath("src"), capture_output=True)
    if make_job.stdout:
        print("Make Job:")
        print(make_job.stdout.decode())

    if make_job.stderr:
        print("Make Err:")
        print(make_job.stderr.decode())

def setup_dam():
    dam_job = subprocess.run(["cargo", "build", "--profile", "release"], check=True, cwd=dam_path, capture_output=True)

    if dam_job.stdout:
        print("DAM Job:")
        print(dam_job.stdout.decode())

    if dam_job.stderr:
        print("DAM Err:")
        print(dam_job.stderr.decode())

def run_sst(threads: int, fib_size: int, iters: int, depth: int, trees: int, latency: int, chan_depth: int, imbalance: int):
    cmd = ["/usr/bin/time", "-f", "%e", "sst", "benchmark.py", f"-n {threads}", "--", f"--fib-size={fib_size}", f"--iters={iters}", f"--depth={depth}",
                          f"--num-trees={trees}", f"--latency={latency}ns", f"--channel-depth={chan_depth}", f"--imbalance={imbalance}"]
    print(" ".join(cmd))
    job = subprocess.run(cmd, capture_output=True, cwd=sst_path.joinpath("tests"))
    return job

def run_dam(fib_size: int, iters: int, depth: int, trees: int, latency: int, chan_depth: int, imbalance: int, fifo: bool):
    cmd = ["/usr/bin/time", "-f", "%e", "target/release/dam-compare", f"--fib-size={fib_size}", f"--iters={iters}", f"--depth={depth}",
                          f"--num-trees={trees}", f"--latency={latency}", f"--channel-depth={chan_depth}", f"--imbalance={imbalance}"]
    if fifo:
        cmd.append("--fifo-mode")
    print(" ".join(cmd))
    job = subprocess.run(cmd, capture_output=True, cwd=dam_path)
    return job

def parse_output(job: subprocess.CompletedProcess):
    seconds = float(job.stderr.decode())
    return seconds

# Sweep space
num_iters = [100000]
fib_size = [10, 12, 14]
num_trees = [1, 10, multiprocessing.cpu_count()]
imbalance = [0, 1, 2, 3, 4]
depths = [10, 11, 12]

if __name__ == "__main__":
    setup_sst()
    setup_dam()
    
    sst_results = collections.defaultdict(list)
    dam_results = collections.defaultdict(list)

    repeats = 5

    for (iters, fib, trees, imba, depth) in itertools.product(num_iters, fib_size, num_trees, imbalance, depths):
        for _ in range(repeats):
            sst_results[(iters, fib, trees, imba, depth)].append(parse_output(run_sst(multiprocessing.cpu_count(), fib, iters, depth, trees, 1, 1024, imba)))

        for _ in range(repeats):
            dam_results[(iters, fib, trees, imba, depth)].append(parse_output(run_dam(fib, iters, depth, trees, 1, 1024, imba, False)))

    # sst_result = run_sst(20, 10, 1000, 10, 20, 1, 1024, 3)
    # dam_result1 = run_dam(20, 10, 1000, 10, 20, 1, 1024, 3, False)
    # dam_result2 = run_dam(20, 10, 1000, 10, 20, 1, 1024, 3, True)
