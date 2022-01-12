import os

"""
reverse = {ip: node_id}
fibs = {
    timestamp: {
        node_id: {
            neighs: [(a, b)], # a via b
            routes: [a, b]  # a via b
    }
}
"""

def ip_into_rid(ip: str) -> int:
    return int(ip.split('.')[-1]) - 1

def parse_route(route: list) -> list:
    return [ip_into_rid(nexthop) for nexthop in route]

def collect_fibs(pwd, fibs, reverse):
    for entry in os.listdir(pwd):
        if entry.isdigit():
            new_pwd = "%s/%s" % (pwd, entry)
            with open("%s/cmdline" % new_pwd) as cmdline:
                if "ip r" not in cmdline.read():
                    continue
            with open("%s/status" % new_pwd) as status:
                content = status.read()

            a = [i for i in content.split('\n')[0].split(' ') if i != '']

            """
            filter out eventual dumps happening before the failure.
            this should not happen since they are removed to lighten the logs.
            """
            if int(a[4][:-1]) < 60:
                continue

            timestamp = float(a[6][1:-3]) / 1000000
            #if timestamp == 60000.0:
            #    continue

            with open("%s/stdout" % new_pwd) as stdout:
                raw_fib = stdout.read()
            raw_fib = [[j for j in i.split(' ') if j != ''] for i in raw_fib.split('\n') if i != '']
            neighs = {}
            routes = {}
            current = None
            for entry in raw_fib:
                if entry[4] == 'kernel':
                    # neighbors
                    neighs[entry[0]] = entry[-1]
                    current = None
                elif entry[1] == 'dev' and entry[2] == 'lo':
                    # lo address == node id
                    node_id = entry[0]
                    current = None
                elif entry[1] == 'via' and entry[0] != '\tnexthop':
                    routes[entry[0]] = [(entry[2], True)]
                    current = None
                elif entry[1] == 'via' and entry[0] == '\tnexthop':
                    fib_entry = (entry[2], not (entry[-1] == 'linkdown' and entry[-2] == 'dead'))
                    try:
                        routes[current].append(fib_entry)
                    except KeyError:
                        routes[current] = [fib_entry]
                elif entry[1] == 'proto' and entry[2] == 'bird':
                    current = entry[0]
                else:
                    print("Unexpected entry", entry)
            try:
                fibs[timestamp][node_id] = {'neighs': neighs, 'routes': routes}
            except KeyError:
                fibs[timestamp] = {node_id: {'neighs': neighs, 'routes': routes}}

            for a, b in neighs.items():
                # a via b
                reverse[b] = node_id

    return (fibs, reverse)

def explore(src: str, dst: str, fibs: dict, reverse: dict) -> list:
    from copy import deepcopy

    def loop(src: str, dst: str, fibs: dict, reverse: dict, route: list, collect) -> list:
        route.append(src)
        if src == dst:
            collect.append(parse_route(route))
            return

        try:
            nexthop_linklocal = fibs[src]['routes'][dst]
        except KeyError:
            collect.append(("%i not found in <%s> fib: %s" % (ip_into_rid(dst), ip_into_rid(src), fibs[src]['routes'])))
            return

        #print(nexthop_linklocal)

        for (candidate, status) in nexthop_linklocal:
            if not status:
                print('linkdown in fib')
                continue

            try:
                nexthop = reverse[candidate]
            except KeyError as e:
                # not expected
                print(e, reverse.keys())
                continue

            #print(ip_into_rid(nexthop))
            if nexthop in route:
                collect.append(("loop", ip_into_rid(nexthop), parse_route(route)))
                continue

            #route.append(nexthop)
            loop(nexthop, dst, fibs, reverse, deepcopy(route), collect)

    collect = []
    loop(src, dst, fibs, reverse, [], collect)
    print("%i -> %i" % (ip_into_rid(src), ip_into_rid(dst)), collect, end=' ')
    return collect

def evaluate_convergence(pwd): 
    def instant_of_convergence(raw_data):
        data = [(t, raw_data[t]) for t in sorted(raw_data, reverse=True)]
        for idx, (timetsamp, value) in enumerate(data):
            if not value:
                candidate = data[idx-1]
                return candidate[0] if candidate[1] else -1
        return data[-1][0] if data[-1][1] else -1

    print('Evaluate convergence for %s' % pwd)
    fibs = {}
    reverse = {}
    nodes = os.listdir(pwd)
    n_nodes = len(nodes)
    result = {}
    for node in nodes:
        fibs, reverse = collect_fibs('%s/%s/var/log' % (pwd, node), fibs, reverse)

    fibs = {timestamp: fibs[timestamp] for timestamp in sorted(fibs.keys())}

    for timestamp, data in fibs.items():
        print(timestamp)
        correct = 0
        incorrect = 0
        for i in range(1, n_nodes+1):
            for j in range(1, n_nodes+1):
                if i == j:
                    continue
                src = '10.0.1.%i' %i
                dst = '10.0.1.%i' %j

                routes = explore(src, dst, data, reverse)
                valid = len([item for item in routes if type(item) == list]) == len(routes) and len(routes) > 0
                print(valid)
                
                #print(route if route is not None else "No route from <%s> to <%s>. %s" % (src, dst, reason))
                if valid:
                    correct += 1
                else:
                    incorrect += 1
                #print(route if route is not None else "No route %i -> %i. %s" % (ip_into_rid(src), ip_into_rid(dst), reason))
            print()

        result[timestamp] = correct == n_nodes*(n_nodes-1)
        if correct + incorrect != n_nodes*(n_nodes-1):
            print('ERROR: cumulated sum of correct and incorrect routes does not correspond total number of expected routes')
            return (0, -1)

    print(result)
    ioc = instant_of_convergence(result)
    print(ioc, '\n')
    rid = pwd.split('/')[2]
    return (rid, ioc)

import sys
pwd = "output/%s" % sys.argv[1]
results = {}
for failure in os.listdir(pwd):
    failure_id, value = evaluate_convergence("%s/%s/files" % (pwd, failure))
    if value < 0:
        print("No valid result for %s" % failure_id)
        continue
    results[failure_id] = (value * 1000 - 60000000) / 1000
print(results)
