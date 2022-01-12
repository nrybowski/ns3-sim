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
VALIDATE = 500 / (dx/1000)

def iconvert(timestamp: int) -> int:
    return timestamp*pow(10,6) + ORIGIN

def convert(timestamp: int) -> float:
    return (timestamp - ORIGIN) / pow(10, 6)

def local_gaps(data: str):
    fails = [60+i*10 for i in range(100)]
    gaps = []
    control = {}
    for line in data.split('\n')[:-1]:
        src, timestamp, received = line.split(', ')
        timestamp = int(timestamp)
        received = int(received)
        try:
            # low == 0: init phase, previous != 0 => candidate search phase
            if (control[src]['low'] == 0 or timestamp - control[src]['low'] == dx) and control[src]['previous'] == 0:
                control[src]['low'] = timestamp
                control[src]['low_received'] = received
            else:
                if control[src]['previous'] != 0:
                    # we are in candidate search phase
                    if timestamp - control[src]['previous'] == dx:
                        # correct timestamp succession found
                        if control[src]['candidate'] == 0:
                            # no candidate already set => candidate is the previous timestamp
                            control[src]['candidate'] = control[src]['previous']
                            control[src]['high_received'] = control[src]['previous_received']
                        else:
                            # we found a correct timestamp succession and we already have a candidate, validation phase
                            #print(timestamp, control[src]['previous'])
                            if control[src]['validate'] == 0:
                                # validation phase finished, candidate is the higher bound
                                # gap is candidate - low
                                tmp = [
                                    (int(src.split('.')[-1]) - 1),
                                    control[src]['low'],
                                    control[src]['high_received'],
                                    (control[src]['high_received'] - control[src]['low_received'])/1000,
                                    #control[src]['candidate'],
                                    #(control[src]['candidate'] - control[src]['low'])/1000,
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
                                # reset all state
                                control[src]['candidate'] = 0
                                control[src]['high_received'] = 0
                                control[src]['previous'] = 0
                                control[src]['validate'] = VALIDATE
                                control[src]['low'] = timestamp
                                control[src]['low_received'] = received
                                continue
                            else:
                                control[src]['validate'] -= 1
                    else:
                        # an incorrect timestamp succession is found, reset the candidate
                        control[src]['candidate'] = 0
                        control[src]['high_received'] = 0
                        print('%s reset candi %i %i' % (src, timestamp, control[src]['previous']))
                # candidate search phase, low is the lower bound and previous takes its role
                control[src]['previous'] = timestamp
                control[src]['previous_received'] = received
        except KeyError:
            control[src] = {'low': timestamp, 'previous': 0, 'candidate': 0, 'validate': VALIDATE}
    a = {}
    idx = 0
    def fit(fail, gap):
        f = iconvert(fail)
        return f >= gap[1] and f <= gap[2]

    for gap in gaps:
        try:
            back = idx
            while not fit(fails[idx], gap):
                idx +=1
            try:
                a[fails[idx]].append(gap)
            except KeyError:
                a[fails[idx]] = [gap]
        except IndexError:
            print('Unexpected: ', gap)
            idx = back
            continue

    #print(a)

    return a

def converge_time(base: str):
    all_gaps = {}
    for directory in os.listdir(base):
            #directory = 'files-4'
        with open('%s/%s/var/log/udp_ping.log' % (base, directory), 'r') as fd:
            print(directory)
            for key, value in local_gaps(fd.read()).items():
                try:
                    all_gaps[key] += value
                except KeyError:
                    all_gaps[key] = value
    #print(all_gaps)
    #print()
    convergence = {}
    for key, value in all_gaps.items():
        low = sorted(value, key=lambda x: x[1])[0][1]
        high = sorted(value, key=lambda x: x[2], reverse=True)[0][2]
        print(key, convert(low), convert(high), (high - low)/1000)
        convergence[key] = int((high - low)/1000)
    #print(convergence)
    return convergence

if __name__ == '__main__':
    import sys
    data = {}
    for base in os.listdir(sys.argv[1]):
        a = os.path.join(sys.argv[1], base)
        if not os.path.isdir(a):
            continue
        print(base)
        basedir = '%s/%s/files' % (sys.argv[1], base)
        ret = converge_time(basedir)
        try:
            data[base] = ret[60]
        except KeyError:
            print('No gap')
    print(data)
    x = [x[1] for x in sorted(data.items(), key=lambda x: x[1])]

    import matplotlib.pyplot as plt
    plt.plot(x, 'bx')
    plt.ylabel('Convergence Time [ms]')
    plt.xlabel('Link Failures')
    plt.title('Sorted convergence time for GEANT links')
    plt.savefig('plot.png', dpi=600)

    """

    #data = {60: 340, 70: 385, 80: 285, 100: 350, 110: 385, 120: 455, 130: 295, 150: 480, 170: 425, 180: 405, 90: 270, 240: 230, 310: 335, 210: 390, 250: 245}

    #x = [i for i in range(60, 311, 10)]
    x = [i for i in range(60, 131, 10)]
    xi = list(range(len(x))) 
    y = [data[i] if i in data else 0 for i in x]

    plt.plot(xi, y, 'bo')
    plt.plot(xi, y)
    plt.xticks(xi, x)
    plt.ylabel('convergence time [ms]')
    plt.xlabel('failures timestamps [s]')
    plt.show()
    """
