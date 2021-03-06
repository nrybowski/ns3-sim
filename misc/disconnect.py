import sys

from scipy.sparse import csr_matrix, bsr_matrix
from scipy.sparse.csgraph import dijkstra
import numpy as np

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
graph = []
for idx, node in enumerate(nodes):
    graph.append([links[node][nodes[i]] if nodes[i] in links[node] else np.inf for i in range(0, n)])

print(graph)
print(np.array_str(np.array(graph)))

#dist_matrix, predecessors = dijkstra(csgraph=csr_matrix(np.array(graph)), return_predecessors=True)
#_, predecessors = dijkstra(csgraph=csr_matrix(np.array(graph)), return_predecessors=True)
#print(predecessors)
