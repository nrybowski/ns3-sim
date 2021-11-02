#! /usr/bin/python3

def get_metric(node_id, iface_id):
    with open('files-%s/etc/bird.conf' % node_id) as fd:
        config = fd.read()

    lines = config.split('\n')[:-1]
    for lid, line in enumerate(lines):
        if 'sim%s' % iface_id in line:
            costs = [line for line in lines[lid:lid+5] if 'cost' in line]
            if len(costs) > 1:
                print('error')
                exit(1)
            metric = costs[0].split(' ')[-1][:-1]
    return metric

def nodemap(filename: str) -> dict:
    ret = {}
    with open(filename, 'r') as fd:
        data = fd.read()
    for line in data.split('\n')[:-1]:
        node_id, name = line.split(' ')
        ret[node_id] = name
    return ret

if __name__ == '__main__':
    with open('geant.check', 'r') as fd:
        raw = fd.read()
    csv = raw.split('\n')[:-1]

    nodes = nodemap("geant.map")
    
    generated_topo = []
    for line in csv:
        n1, i1, n2, i2, delay = line.split(',')
        generated_topo.append('%s %s %s %s' % (nodes[n1], nodes[n2], get_metric(n1, i1), delay))

    with open('geant.ntf', 'r') as fd:
        ntf = fd.read()
    for line in ntf.split('\n')[:-1]:
        if line[0] == '#':
            continue

        try:
            generated_topo.remove(line)
        except ValueError:
            print('ERROR: <%s> link not found in generated topo' % line)
            exit(1)

    if (len(generated_topo) > 0):
        print('ERROR: generated topo contains more links than NTF file', generated_topo)
        exit(1)

    print('Success: generated topo corresponds to NTF file')

