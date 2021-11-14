from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra
import numpy as np

with open('networks/house/house.ntf', 'r') as fd:
    data = fd.read().split('\n')[:-1]

nodes = []
links = {}

for line in data:
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

#print(csr_matrix(graph))

#dist_matrix, predecessors = dijkstra(csgraph=csr_matrix(np.array(graph)), return_predecessors=True)
_, predecessors = dijkstra(csgraph=csr_matrix(np.array(graph)), return_predecessors=True)
print(predecessors)

leaves = [['10.0.1.%i' % (i+1) for i in range(0,6) if i not in pred] for pred in predecessors]

with open('udp.ping', 'w') as fd:
    for line in leaves:
        fd.write('%s\n' % ', '.join(line))
