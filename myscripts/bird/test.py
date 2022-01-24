#! /usr/bin/python3

import os
import sys 
from re import sub

metrics = {}

def get_metric(node_id, iface_id):
    global metrics

    try:
        return metrics[node_id][iface_id]
    except KeyError:
        pass

    # read <node_id> config file to get back the link metrics
    with open('files-%s/etc/bird.conf' % node_id) as fd:
        config = fd.read()

    last = -1
    for line in config.split('\n'):
        # skip empty line
        if line == '': continue

        # remove tabulations and split line by spaces
        line = [i for i in sub('\t', '', line).split(' ') if i != '']
        if line[0] == 'interface' and line[1][:4] == '"sim':
            # 'sim*' interface configuration block
            current_iface_id = line[1][4:-1]
            if last == -1:
                # parse new interface
                last = current_iface_id
            else:
                # error, we try to parse a new interface while the previous one is not completed
                print('ERROR: try to parse %s while %s is not finished' % (current_iface_id, last))
                exit(1)
        elif line[0] == 'cost' and len(line) == 2:
            # got the cost of the link seen by an interface
            metric = line[1][:-1] 
            if last != -1:
                try:
                    metrics[node_id][last] = metric
                except KeyError:
                    metrics[node_id] = {last: metric}
                last = -1
            else:
                print('ERROR: got a metric not associated to an interface')
                exit(1)

    return metrics[node_id][iface_id]

def nodemap(filename: str) -> dict:
    """ Collect nodemap in dict
    """
    ret = {}
    with open(filename, 'r') as fd:
        data = fd.read()
    for line in data.split('\n')[:-1]:
        node_id, name = line.split(' ')
        ret[node_id] = name
    return ret

# check => map => ntf
if __name__ == '__main__':
    with open(sys.argv[1], 'r') as fd:
        raw = fd.read()
    csv = raw.split('\n')[:-1]

    nodes = nodemap(sys.argv[2])
    
    generated_topo = []
    for line in csv:
        n1, i1, n2, i2, delay = line.split(',')
        msg = '%s %s %s %s' % (nodes[n1], nodes[n2], get_metric(n1, i1), delay)

        generated_topo.append(msg)

    with open(sys.argv[3], 'r') as fd:
        ntf = fd.read()
    ntf = ntf.split('\n')
    for line in ntf:
        if line == '' or line[0] == '#':
            continue
        splitted = line.split(' ')
        new_line = ' '.join(splitted[:-1] + [format(float(splitted[-1]), '.6f')])
        try:
            generated_topo.remove(new_line)
        except ValueError:
            print('ERROR: <%s> link not found in generated topo' % new_line)
            for item in generated_topo:
                if ' '.join(line.split(' ')[:2]) in item:
                    print(item)

            exit(1)

    if (len(generated_topo) > 0):
        print('ERROR: generated topo contains more links than NTF file', generated_topo)
        exit(1)

    print('Success: generated topo corresponds to NTF file')

