import matplotlib.pyplot as plt
import numpy as np

f = open("tcp-example.tr", "r")
data = f.readlines()
# data = data[:100]

xVal = []
yVal = []

q = {}
total_delay = 0
total_packets = 0

for line in data:
    arr = line.strip().split()

    seq = int(arr[36][4:])
    time = float(arr[1])
    enq = arr[0] == '+'
    deq = arr[0] == '-'

    if enq:
        q[seq] = time
    elif deq:
        if seq not in q.keys():
            continue
        delay = time - q[seq]
        total_delay += delay
        total_packets += 1

        if (len(xVal) > 0 and xVal[-1] == time):
            yVal[-1] = (delay)
        else:
            xVal.append(time)
            yVal.append(delay)
        q.pop(seq)

xpoints = np.array(xVal)
ypoints = np.array(yVal)

plt.plot(xpoints, ypoints * 1000, '.')
plt.xlabel('Time (s)')
plt.ylabel('Queuing Delay (ms)')
# plt.show()
plt.savefig("Instantaneous.png")
