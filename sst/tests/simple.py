import sst

obj = sst.Component("sstbench", "sstbench.SimpleExternalElement")
obj.addParams({
    "printFrequency" : "5",
    "repeats" : "15"
    })