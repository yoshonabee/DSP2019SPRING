import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

with open(sys.argv[1], 'r') as f:
	reader = csv.reader(f, delimiter=',')
	data = [[float(row[0]), float(row[1])] for row in reader]

data = np.array(data)

plt.xlabel('# of iteration (+x)')
plt.ylabel('acc')
plt.plot(data[:,0], data[:,1], marker='o', markersize=8, color='orange')
plt.ylim(90, 100)
plt.show()

