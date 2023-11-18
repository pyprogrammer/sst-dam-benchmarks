import sst
import itertools

import sys

import argparse

import time

start = time.time()

parser = argparse.ArgumentParser()
parser.add_argument("--fib-size", type=int)
parser.add_argument("--iters", type=int)
parser.add_argument("--depth", type=int)
parser.add_argument("--num-trees", type=int)
parser.add_argument("--latency", type=str)
parser.add_argument("--channel-depth", type=int)
parser.add_argument("--imbalance", type=int, default=1)  # How much to multiply the fibonacci factor for the first tree.

args = parser.parse_args(sys.argv[1:])

node_counter = itertools.count()
link_counter = itertools.count()

print(args)

width = 2**args.depth

for tree_ind in range(args.num_trees):
    input_streams = [
        sst.Component(f"signal_{next(node_counter)}", "sstbench.SignalGenerator") for _ in range(width)
    ]
    for stream in input_streams:
        stream.addParams({"repeats": args.iters, "capacity": args.channel_depth})

    while len(input_streams) > 1:
        new_streams = []
        pairs = zip(input_streams[::2], input_streams[1::2])
        for i, j in pairs:
            adder = sst.Component(f"sum_{next(node_counter)}", "sstbench.SumWithFibonacci")
            adder.addParams({"capacity": args.channel_depth, "fib": args.fib_size + args.imbalance if tree_ind == 0 else args.fib_size, "repeats": args.iters})
            link = sst.Link(f"link_{next(link_counter)}")
            link.connect((i, "output_link", args.latency), (adder, "inputA", args.latency))

            link2 = sst.Link(f"link_{next(link_counter)}")
            link2.connect((j, "output_link", args.latency), (adder, "inputB", args.latency))
            new_streams.append(adder)
        input_streams = new_streams

    clink = sst.Link(f"Final_{next(link_counter)}")
    checker = sst.Component(f"checker_{next(node_counter)}", "sstbench.Checker")
    checker.addParams({"repeats": args.iters})
    clink.connect(
        (input_streams[0], "output_link", args.latency), (checker, "input_link", args.latency)
    )

print("Number of Links:", next(link_counter))
print("Number of Contexts:", next(node_counter))
setup_time = time.time() - start
print("Setup Time (s):", setup_time)