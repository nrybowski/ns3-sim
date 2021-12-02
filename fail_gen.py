with open('inputs/geant.ntf', 'r') as fd:
    data = fd.read()

r = [line.split(' ') for line in data.split('\n')[:-1] if line[0] != '#']
b = [line for line in r]
print(b)
for line in b:
    print(line)
    del b[b.index([line[1], line[0], line[2], line[3]])]
print(b.sort(key=lambda x: int(x[2])))
with open('geant-failures.ntf', 'w') as fd:
    for line in b:
        fd.write(' '.join(line)+'\n')
"""    
a = sorted(r, key=lambda x: int(x[2]))
j = 0
print(r)
print([i for j, i in enumerate(r) if j %2 == 0])
"""
