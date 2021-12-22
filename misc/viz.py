from numpy import inf
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra
import graphviz


network = [[inf, 1, 10, inf, inf, inf], [1, inf, 1, 1, 10, inf], [10, 1, inf, inf, 1, 1], [inf, 1, inf, inf, inf, 1], [inf, 10, 1, inf, inf, inf], [inf, inf, 1, 1, inf, inf]]
_, forest = dijkstra(csgraph=csr_matrix(network), return_predecessors=True)

def gen_graphs(forest: list):
    for root, tree in enumerate(forest):
        dot = graphviz.Digraph()
        for child, parent in enumerate(tree):
            if parent != -9999:
                dot.edge(str(parent), str(child))
        dot.render('tree-%i' % root, view=True)


