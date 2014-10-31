import matplotlib.pyplot as plt
import numpy as np
import pylab

import matplotlib.pyplot as plt
import numpy as np
import pylab

numberOfSchedulers = 5
experiment = "DataParallelExperiment-thread"
inFile = experiment+".dat"

f = open(inFile, 'r')

f.readline()
line = f.readline()
t = line.split()

xLabel = t[1]

yThroughputLabels = []
yLatencyLabels = []

for i in xrange(2, 2+numberOfSchedulers*4, 4):
	yThroughputLabels.append(t[i])
	yLatencyLabels.append(t[i+2])

for i in xrange (0, numberOfSchedulers, 1):
	print yThroughputLabels[i]
print
for i in xrange (0, numberOfSchedulers, 1):
	print yLatencyLabels[i]

line = f.readline()
x = []

yThroughputValues = []
yThroughputDeviations = []
yLatencyValues = []
yLatencyDeviations = []

for i in xrange(0, numberOfSchedulers):
	yThroughputValues.append([])
	yThroughputDeviations.append([])
	yLatencyValues.append([])
	yLatencyDeviations.append([])

while (line) :
    t = line.split()
    x.append(float(t[0]))
    for i in xrange(1, numberOfSchedulers*4+1, 4):
    	yThroughputValues[(i-1)/4].append(float(t[i]));
    	yThroughputDeviations[(i-1)/4].append(float(t[i+1]));
    	yLatencyValues[(i-1)/4].append(float(t[i+2]));
    	yLatencyDeviations[(i-1)/4].append(float(t[i+3]));

    line = f.readline()

x = np.array(x)
yThroughputValues = np.array(yThroughputValues)
yThroughputDeviations = np.array(yThroughputDeviations)
yLatencyValues = np.array(yLatencyValues)
yLatencyDeviations = np.array(yLatencyDeviations)

lines = ["bo-", "rs-", "g*-", "m+-", "c^-"]
colors = ["b", "r", "g", "m", "c"]
markers = ["o", "s", "*", "+", "^"]

plt.figure()
for i in xrange(0, numberOfSchedulers):
	plt.errorbar(x, yThroughputValues[i], xerr=0, yerr=yThroughputDeviations[i], marker = markers[i], color = colors[i], label=yThroughputLabels[i])
plt.legend(loc='best', shadow=False)
plt.xlabel(xLabel)
plt.ylabel('throughput')
plt.title(experiment);
plt.savefig(experiment+'throughput')
plt.close()

for i in xrange(0, numberOfSchedulers):
	plt.errorbar(x, yLatencyValues[i], xerr=0, yerr=yLatencyDeviations[i], marker = markers[i], color = colors[i], label=yLatencyLabels[i])
plt.legend(loc='best', shadow=False)
plt.xlabel(xLabel)
plt.ylabel('latency')
plt.title(experiment);
plt.savefig(experiment+'latency')

