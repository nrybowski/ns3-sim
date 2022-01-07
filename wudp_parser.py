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
            if int(a[4][:-1]) >= 60:
                timestamp = float(a[6][1:-3]) / 1000000
                with open("%s/stdout" % new_pwd) as stdout:
                    raw_fib = stdout.read()
                raw_fib = [[j for j in i.split(' ') if j != ''] for i in raw_fib.split('\n') if i != '']
                neighs = {}
                routes = {}
                for entry in raw_fib:
                    if entry[4] == 'kernel':
                        # neighbors
                        neighs[entry[0]] = entry[-1]
                    elif entry[1] == 'dev' and entry[2] == 'lo':
                        # lo address == node id
                        node_id = entry[0]
                    elif entry[1] == 'via':
                        routes[entry[0]] = entry[2]
                    else:
                        print("Unexpected entry", entry)
                try:
                    fibs[timestamp][node_id] = {'neighs': neighs, 'routes': routes}
                    for a, b in neighs.items():
                        # a via b
                        reverse[b] = node_id
                except KeyError:
                    fibs[timestamp] = {node_id: {'neighs': neighs, 'routes': routes}}

    return (fibs, reverse)

def explore(src, dst, fibs, reverse, route) -> ():
    try:
        nexthop_linklocal = fibs[src]['routes'][dst]
        #print("nexthop_ll", nexthop_linklocal)
    except KeyError:
        return (None, "<%s> not found in <%s> fib: no route to host" % (dst, src))

    nexthop = reverse[nexthop_linklocal]
    #print("nexthop", nexthop)

    if nexthop in route:
        return (None, ("loop", nexthop, route))

    route.append(nexthop)

    if nexthop == dst:
        return (route,"")
    else:
        return explore(nexthop, dst, fibs, reverse, route)

def evaluate_convergence(pwd): 
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
        correct = 0
        for i in range(1, n_nodes+1):
            for j in range(1, n_nodes+1):
                if i == j:
                    continue
                src = '10.0.1.%i' %i
                dst = '10.0.1.%i' %j

                route, reason = explore(src, dst, data, reverse, [src])
                
                #print(route if route is not None else "No route from <%s> to <%s>. %s" % (src, dst, reason))
                if route is not None:
                    correct += 1

        result[timestamp] = correct == n_nodes*(n_nodes-1)
    print(result, '\n')

pwd = "output/house.ntf"
for failure in os.listdir(pwd):
    evaluate_convergence("%s/%s/files" % (pwd, failure))






