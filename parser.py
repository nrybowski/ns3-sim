import sys

#ORIGIN = 1262303999000000
ORIGIN = 1262304000000000

def epoch_to_relative(to_convert: int) -> float:
    return (float(to_convert - ORIGIN) / pow(10,6))

if len(sys.argv) < 2:
    print("Usage:")
    exit(1)

path = sys.argv[1]

with open(path, 'r') as fd:
    data = fd.read().split('\n')[:-1]

last = {}

print(path)

""" src_id, received_time, current_time """

for line in data:
    rid, recv, current = line.split(',')
    recv = int(recv)
    current = int(current)
    try:
        """ gap = last stored - last received """
        
        if last[rid]['cg'] != 0:
            gap = recv - last[rid]['clv']
        else:
            gap = recv - last[rid]['lv']

        cut_rid = int(rid.split('.')[-1]) - 1
        received = epoch_to_relative(recv)
        current = epoch_to_relative(current)
        

        if gap == 5000:
            if last[rid]['cg'] != 0:
                print('%i %f %f %i' % (cut_rid, epoch_to_relative(last[rid]['clv']), epoch_to_relative(last[rid]['lv']), last[rid]['cg'] / 1000))
                last[rid]['cg'] = 0
                last[rid]['clv'] = 0
            last[rid]['lv'] = recv
        else:
            if gap < 0:
                print('%i micro-loop' % cut_rid)
            last[rid]['cg'] = recv - last[rid]['lv']
            last[rid]['clv'] = recv

    except KeyError:
        last[rid] = {'lv': recv, 'clv': 0, 'cg': 0}
