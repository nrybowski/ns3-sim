import sys

if len(sys.argv) < 2:
    print("Usage:")
    exit(1)

path = sys.argv[1]

with open(path, 'r') as fd:
    data = fd.read().split('\n')[:-1]

content = {}

print(path)

for line in data:
    rid, recv, current = line.split(',')
    recv = int(recv)
    try:
        gap = recv - content[rid]
        if gap != 5000:
            print("%i <%i> %i" % (int(rid.split('.')[-1]) - 1 , (gap/1000), content[rid]))
        content[rid] = recv
    except KeyError:
        content[rid] = recv
