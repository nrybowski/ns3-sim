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
    entry = {'f_metric': int(metric), 'delay': int(float(delay)*1000), 'r_metric': -1, 'begin': int(begin), 'end': int(end)}

    try:
        link = links[key]
        if link[-1]['r_metric'] != -1:
            links[key].append(entry)
        else:
            print('#unexpected')
            exit(1)
    except KeyError:
        # no forward
        try:
            # got reverse
            link = links[tuple([key[i-1] for i in range(len(key), 0, -1)])]
            link[-1]['r_metric'] = int(metric)
        except KeyError:
            # no reverse => new link
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

id_src = mapping if len(mapping) > 0 else nodes
for (src, dst), l in links.items():
    for link in l:
        print('%i %i %i %i %i %i %i' % (id_src.index(src), id_src.index(dst), link['f_metric'], link['r_metric'], link['delay'], link['begin'], link['end']))

#fd.flush()
#fd.close()
#print(filepath)
