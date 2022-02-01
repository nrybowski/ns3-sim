import sys
import os

if len(sys.argv) < 2:
    print('#Usage %s' % sys.argv[0])
    exit(1)

filename = sys.argv[1]

with open('%s/%s' % (os.environ.get('PWD'), filename), 'r') as fd:
    data = fd.read()


"""
{
    (src, dst): [
        {
            'f_metric': <f_metric>,
            'r_metric': <r_metric>,
            'delay': <delay>,
            'begin': <begin>,
            'end': <end>

        }
    ]
}
"""
nodes = []
links = {}

for line in data.split('\n'):
    if line == '' or line[0] == '#':
        continue

    split = line.split(' ')
    src, dst, metric, delay, begin, end = split if len(split) == 6 else split + [-1 for i in range(6-len(split))]
    key = (src, dst)
    entry = {'f_metric': int(metric), 'delay': round(float(delay)*1000000), 'r_metric': -1, 'begin': int(begin), 'end': int(end)}

    try:
        link = links[key]
        if link[-1]['r_metric'] != -1 and link[-1]['f_metric'] != -1:
            # link already parsed in both direction, we got a new link
            links[key].append(entry)
        elif link[-1]['r_metric'] != -1:
            # link already parsed in reverse direction, we set the forward metric
            link[-1]['f_metric'] = entry['f_metric']
        else:
            print('#unexpected')
            exit(1)
    except KeyError:
        # no forward
        try:
            # got reverse
            reversed_key = tuple([key[i-1] for i in range(len(key), 0, -1)])
            link = links[reversed_key]
            if link[-1]['r_metric'] == -1:
                # unset reverse of existing forward
                link[-1]['r_metric'] = int(metric)
            else:
                # revsere and forward already set, got a new link
                entry['r_metric'] = entry['f_metric']
                entry['f_metric'] = -1
                links[reversed_key].append(entry)
        except KeyError:
            # no forward nor reverse, we got a new link
            links[key] = [entry]
            for node in key:
                if node not in nodes:
                    nodes.append(node)


n_links = 0
n_pairs = 0

for pair, l in links.items():
    n_links += len(l)
    if len(l) > 1:
        n_pairs += 1

print('#nodes %i' % len(nodes))
print('#links %i (%i)' % (n_links, n_links*2))
print('#pairs %i' % len(links.keys()))
print('#pairs with more than one link %i' % n_pairs)

topo = True

import tempfile
import os

#fd, filepath = tempfile.mkstemp()
#fd = os.fdopen(fd, 'w')

# load node name -> node id mapping if it exists
mapping = []
if "failures" in filename:
    try:
        with open('%s/%s.map' % (os.environ.get('PWD'), filename.split('-')[0])) as fd:
            raw_mapping = fd.read()
        for line in raw_mapping.split('\n'):
            mapping.append(line.split(' ')[-1])
    except FileNotFoundError as e:
        exit(1);
else:
    with open('%s.map' % filename.split('.')[0], 'w') as fd:
        for idx, node in enumerate(nodes):
            fd.write('%i %s\n' % (idx, node))

from numpy import inf, zeros
from scipy.sparse import csr_matrix

mygraph = zeros((len(nodes), len(nodes)))
id_src = mapping if len(mapping) > 0 else nodes
fail = 0
data = [' '.join(j.split(' ')[:-1] + [format(float(j.split(' ')[-1]), '.6f')]) for j in data.split('\n') if j != '' and  j[0] != '#']
for (src, dst), l in links.items():
    src_id = id_src.index(src)
    dst_id = id_src.index(dst)

    for link in l:
                # output for C++ wrapper
        print('%i %i %i %i %i %i %i' % (src_id, dst_id, link['f_metric'], link['r_metric'], link['delay'], link['begin'], link['end']))

        if 'failures' not in filename:
            mygraph[src_id][dst_id] = link['f_metric']
            mygraph[dst_id][src_id] = link['r_metric']

        # 
        for i in ['%s %s %i %.6f' % (src, dst, link['f_metric'], link['delay'] / 1000000), '%s %s %i %f' % (dst, src, link['r_metric'], link['delay'] / 1000000)]:
            try:
                data.remove(i)
            except ValueError:
                print('#Warning ', i)
                print('#Warning ', link['delay'])
                fail += 1

if 'failures' not in filename:
    print('#Failures %i' % fail)
    if len(data) != 0:
        exit(1)

    from scipy.sparse.csgraph import connected_components
    mygraph = csr_matrix(mygraph)
    #with open('connected', 'w') as fd:
    n, labesl = connected_components(mygraph)
    if n != 1:
        exit(1)


#fd.flush()
#fd.close()
#print(filepath)
