import sys

if len(sys.argv) != 2:
    print('Usage %s n' % sys.argv[0])
    exit(1)

n = int(sys.argv[1])

for i in range(n):
    for j in range(n):
        if i == j: continue
        print(i, j, '1' , '5')
