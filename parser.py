with open('udp_ping.log', 'r') as fd:
    data = fd.read().split('\n')[:-1]

content = {}

for line in data:
    rid, addr, recv, current = line.split(',')
    recv = int(recv)
    try:
        gap = recv - content[rid]
        if gap != 5000:
            print("%i <%i> %i" % (int(rid.split('.')[-1]) - 1 , (gap/1000), content[rid]))
        content[rid] = recv
    except KeyError:
        content[rid] = recv
