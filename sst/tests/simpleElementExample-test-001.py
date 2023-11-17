import sst

repeats = str(32)
capacity = "8"
latency = "1ns"
copies = 1

for i in range(copies):
    alink = sst.Link(f"a_{i}")
    blink = sst.Link(f"b_{i}")
    clink = sst.Link(f"c_{i}")
    obj = sst.Component(f"mergeElements_{i}", "mergeElements.MergeElements")
    obj.addParams({"repeats": repeats, "capacity": capacity})

    signal1 = sst.Component(f"signal1_{i}", "mergeElements.SignalGenerator")
    signal1.addParams({"repeats": repeats, "capacity": capacity})

    signal2 = sst.Component(f"signal2_{i}", "mergeElements.SignalGenerator")
    signal2.addParams({"repeats": repeats, "capacity": capacity})

    checker = sst.Component(f"checker_{i}", "mergeElements.Checker")

    print("Created elements:", obj, signal1, signal2)

    alink.connect((signal1, "output_link", latency), (obj, "inputA", latency))

    print("Connected A link")

    blink.connect((signal2, "output_link", latency), (obj, "inputB", latency))

    print("Connected B link")
    clink.connect((obj, "output_link", latency), (checker, "input_link", latency))
