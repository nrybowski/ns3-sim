import sys

if len(sys.argv) != 2:
    print('Usage %s n' % sys.argv[0])
    exit(1)

n = int(sys.argv[1])

for i in range(n):
    src = i
    s = i+1
    dst = 0 if s >= n else s

    print(src, dst, '1' , '5')
    print(dst, src, '1' , '5')
