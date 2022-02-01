import sys

from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra
import numpy as np

def gen_addresses(n: int) -> list:
    high = 0
    low = 1

    addresses = []

    for i in range(n):
            if low > 250:
                    high += 1
                    low = 1
            addresses.append('10.1.%i.%i' % (high, low))
            low += 1

    return addresses

if len(sys.argv) < 2:
    print("Usage:")
    exit(1)

with open(sys.argv[1], 'r') as fd:
    data = fd.read().split('\n')[:-1]

nodes = []
links = {}

for line in data:
    if line[0] == '#':
        continue
    src, dst, metric, delay = line.split(' ')
    if src not in nodes:
        nodes.append(src)
    if dst not in nodes:
        nodes.append(dst)

    try:
        links[src][dst] = int(metric)
    except KeyError:
        links[src] = {dst: int(metric)}

#print(nodes)
n = len(nodes)
addr = gen_addresses(n)
graph = []
for idx, node in enumerate(nodes):
    graph.append([links[node][nodes[i]] if nodes[i] in links[node] else np.inf for i in range(0, n)])
print(graph)

#print(csr_matrix(graph))

#dist_matrix, predecessors = dijkstra(csgraph=csr_matrix(np.array(graph)), return_predecessors=True)
_, predecessors = dijkstra(csgraph=csr_matrix(np.array(graph)), return_predecessors=True)
print(predecessors)

#leaves = [[addr[i] for i in range(len(nodes)) if i not in pred] for pred in predecessors]
leaves = [[addr[i] for i in range(n) if i != j] for j in range(n)]
#leaves = [['10.0.1.%i' % (i+1) for i in range(len(nodes))] for j in range(len(nodes))]

with open('udp.ping', 'w') as fd:
    for line in leaves:
        fd.write('%s\n' % ','.join(line))
