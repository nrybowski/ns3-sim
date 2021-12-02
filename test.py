import json
import os

#with open('data.json', 'r') as fd:
#    data = json.load(fd)
#data = [50, 55, 60, 65, 75, 80, 90, 95, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 165, 160, 170, 175, 180, 185, 190, 195, 200]


#data = """10.0.1.6, 1262304089985000, 1262304089999000
#10.0.1.6, 1262304090140000, 1262304090182000
#10.0.1.6, 1262304090110000, 1262304090182000
#10.0.1.6, 1262304090155000, 1262304090183000
#10.0.1.6, 1262304090125000, 1262304090183000
#10.0.1.6, 1262304090085000, 1262304090183000
#10.0.1.6, 1262304090095000, 1262304090183000
#10.0.1.6, 1262304090100000, 1262304090184000
#10.0.1.6, 1262304090115000, 1262304090185000
#10.0.1.6, 1262304090130000, 1262304090186000
#10.0.1.6, 1262304090145000, 1262304090187000
#10.0.1.6, 1262304090160000, 1262304090188000
#10.0.1.6, 1262304090090000, 1262304090188000
#10.0.1.6, 1262304090105000, 1262304090189000
#10.0.1.6, 1262304090120000, 1262304090190000
#10.0.1.6, 1262304090135000, 1262304090191000
#10.0.1.6, 1262304090150000, 1262304090192000
#10.0.1.6, 1262304090165000, 1262304090193000
#10.0.1.6, 1262304090170000, 1262304090198000
#10.0.1.6, 1262304090175000, 1262304090203000
#10.0.1.6, 1262304090180000, 1262304090208000
#10.0.1.6, 1262304090185000, 1262304090213000
#10.0.1.6, 1262304090190000, 1262304090218000
#10.0.1.6, 1262304090195000, 1262304090223000
#10.0.1.6, 1262304090200000, 1262304090229000
#10.0.1.6, 1262304090205000, 1262304090234000
#"""


dx = 5000
ORIGIN = 1262304000000000
VALIDATE = 100

def iconvert(timestamp: int) -> int:
    return timestamp*pow(10,6) + ORIGIN

def convert(timestamp: int) -> float:
    return (timestamp - ORIGIN) / pow(10, 6)

def local_gaps(data: str):
    fails = [90, 120]
    gaps = []
    control = {}
    for line in data.split('\n')[:-1]:
        src, timestamp, received = line.split(', ')
        timestamp = int(timestamp)
        try:
            if (control[src]['low'] == 0 or timestamp - control[src]['low'] == dx) and control[src]['previous'] == 0:
                control[src]['low'] = timestamp
            else:
                if control[src]['previous'] != 0:
                    if timestamp - control[src]['previous'] == dx:
                        if control[src]['candidate'] == 0:
                            control[src]['candidate'] = control[src]['previous']
                        else:
                            if control[src]['validate'] == 0:
                                tmp = [
                                    (int(src.split('.')[-1]) - 1),
                                    control[src]['low'],
                                    control[src]['candidate'],
                                    (control[src]['candidate'] - control[src]['low'])/1000,
                                    timestamp
                                ]
                                print('%i %f %f %i (reset on %i)' % tuple(tmp))
                                gaps.append(tmp)
                                """
                                print('%i %f %f %i (reset on %i)' % (
                                    (int(src.split('.')[-1]) - 1),
                                    convert(control[src]['low']),
                                    convert(control[src]['candidate']),
                                    (control[src]['candidate'] - control[src]['low'])/1000,
                                    timestamp)
                                )
                                """
                                control[src]['candidate'] = 0
                                control[src]['previous'] = 0
                                control[src]['validate'] = VALIDATE
                                control[src]['low'] = timestamp
                                continue
                            else:
                                control[src]['validate'] -= 1
                    else:
                        control[src]['candidate'] = 0
                control[src]['previous'] = timestamp
        except KeyError:
            control[src] = {'low': timestamp, 'previous': 0, 'candidate': 0, 'validate': VALIDATE}
    a = {}
    idx = 0
    def fit(fail, gap):
        f = iconvert(fail)
        return f >= gap[1] and f <= gap[2]

    for gap in gaps:
        while not fit(fails[idx], gap):
            idx +=1
        try:
            a[fails[idx]].append(gap)
        except KeyError:
            a[fails[idx]] = [gap]
    print(a)

    return a

def converge_time():
    base = 'mnt/files'
    all_gaps = {}
    for directory in os.listdir(base):
        with open('%s/%s/var/log/udp_ping.log' % (base, directory), 'r') as fd:
            print(directory)
            for key, value in local_gaps(fd.read()).items():
                try:
                    all_gaps[key] += value
                except KeyError:
                    all_gaps[key] = value
    print(all_gaps)
    print()
    for key, value in all_gaps.items():
        low = sorted(value, key=lambda x: x[1])[0][1]
        high = sorted(value, key=lambda x: x[2], reverse=True)[0][2]
        print(key, convert(low), convert(high), (high - low)/1000)

if __name__ == '__main__':
    converge_time() 
