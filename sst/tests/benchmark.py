import sst
import itertools

import sys

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--fib-size", type=int)
parser.add_argument("--iters", type=int)
parser.add_argument("--depth", type=int)
parser.add_argument("--num-trees", type=int)
parser.add_argument("--latency", type=str)
parser.add_argument("--channel-depth", type=int)

args = parser.parse_args(sys.argv[1:])

node_counter = itertools.count()
link_counter = itertools.count()

print(args)

width = 2**args.depth

for _ in range(args.num_trees):
    input_streams = [
        sst.Component(f"signal_{next(node_counter)}", "mergeElements.SignalGenerator") for _ in range(width)
    ]
    for stream in input_streams:
        stream.addParams({"repeats": args.iters, "capacity": args.channel_depth})

    while len(input_streams) > 1:
        new_streams = []
        pairs = zip(input_streams[::2], input_streams[1::2])
        for i, j in pairs:
            adder = sst.Component(f"sum_{next(node_counter)}", "mergeElements.SumElements")
            adder.addParams({"capacity": args.channel_depth, "fib": args.fib_size})
            link = sst.Link(f"link_{next(link_counter)}")
            link.connect((i, "output_link", args.latency), (adder, "inputA", args.latency))

            link2 = sst.Link(f"link_{next(link_counter)}")
            link2.connect((j, "output_link", args.latency), (adder, "inputB", args.latency))
            new_streams.append(adder)
        input_streams = new_streams

    clink = sst.Link(f"Final_{next(link_counter)}")
    checker = sst.Component(f"checker_{next(node_counter)}", "mergeElements.Checker")
    clink.connect(
        (input_streams[0], "output_link", args.latency), (checker, "input_link", args.latency)
    )

    

# input_streams = [
#     sst.Component(f"signal_{i}", "mergeElements.SignalGenerator") for i in range(width)
# ]
# for stream in input_streams:
#     stream.addParams({"repeats": repeats, "capacity": capacity})

# while len(input_streams) > 1:
#     new_streams = []
#     pairs = zip(input_streams[::2], input_streams[1::2])
#     for i, j in pairs:
#         adder = sst.Component(f"sum_{next(node_counter)}", "mergeElements.SumElements")
#         adder.addParams({"capacity": capacity})
#         link = sst.Link(f"link_{next(link_counter)}")
#         link.connect((i, "output_link", latency), (adder, "inputA", latency))

#         link2 = sst.Link(f"link_{next(link_counter)}")
#         link2.connect((j, "output_link", latency), (adder, "inputB", latency))
#         new_streams.append(adder)
#     input_streams = new_streams

# clink = sst.Link("Final")
# checker = sst.Component("checker", "mergeElements.Checker")
# clink.connect(
#     (input_streams[0], "output_link", latency), (checker, "input_link", latency)
# )

# print("Number of Links:", next(link_counter))
# print("Number of Contexts:", next(node_counter))
