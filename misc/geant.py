import matplotlib.pyplot as plt

geant_all = {'15': 460, '11': 445, '14': 480, '23': 450, '30': 210, '27': 445, '21': 435, '7': 505, '28': 215, '4': 445, '10': 235, '9': 525, '17': 490, '31': 475, '32': 520, '20': 480, '35': 225, '18': 460, '34': 500, '19': 445, '33': 460, '16': 460, '22': 470, '0': 510, '24': 505, '5': 495, '8': 460, '3': 490, '6': 485, '25': 460, '13': 430, '1': 480, '29': 215, '2': 450, '26': 460, '12': 440}

geant_no_custom = {'15': 255, '11': 245, '14': 280, '23': 250, '30': 110, '27': 245, '21': 235, '7': 305, '28': 115, '4': 240, '10': 130, '9': 320, '17': 290, '31': 275, '32': 320, '20': 280, '35': 125, '18': 260, '34': 300, '19': 245, '33': 260, '16': 260, '22': 270, '0': 310, '24': 305, '5': 295, '8': 255, '3': 290, '6': 285, '25': 260, '13': 230, '1': 280, '29': 115, '2': 250, '26': 260, '12': 240}

geant_no_delay = {'15': 210, '11': 210, '14': 210, '23': 110, '30': 110, '27': 210, '21': 110, '7': 210, '28': 110, '4': 210, '10': 110, '9': 210, '17': 210, '31': 110, '32': 210, '20': 210, '35': 110, '18': 210, '34': 210, '19': 110, '33': 110, '16': 210, '22': 110, '0': 210, '24': 210, '5': 210, '8': 210, '3': 210, '6': 210, '25': 110, '13': 210, '1': 210, '29': 110, '2': 210, '26': 110, '12': 110}

geant_no_link = {'15': 410, '11': 410, '14': 410, '23': 215, '30': 210, '27': 410, '21': 215, '7': 410, '28': 210, '4': 410, '10': 215, '9': 415, '17': 410, '31': 220, '32': 410, '20': 410, '35': 210, '18': 410, '34': 410, '19': 215, '33': 215, '16': 410, '22': 210, '0': 410, '24': 410, '5': 410, '8': 410, '3': 410, '6': 410, '25': 220, '13': 410, '1': 410, '29': 210, '2': 410, '26': 220, '12': 215}

idx = [x[0] for x in sorted(geant_all.items(), key=lambda x: x[1])]

#plt.plot([x[1] for x in sorted(geant_all.items(), key=lambda x: x[1])], 'bx')
plt.plot([geant_all[key] for key in idx], 'b.-', label='All delays')
plt.plot([geant_no_custom[key] for key in idx], 'r.-', label='No bird delays')
plt.plot([geant_no_link[key] for key in idx], 'g.', label='No link delays')
plt.plot([geant_no_delay[key] for key in idx], 'x-', label='No bird, link delays')
plt.ylabel('Convergence Time [ms]')
plt.xlabel('Link Failures')
plt.title('Sorted convergence time for GEANT links (SPT delay in bird)')
axes = plt.gca()
axes.set_ylim([0,550])
plt.legend()
plt.savefig('plot.png', dpi=600)
plt.clf()

geant_all_no_spt = {'15': 260, '11': 245, '14': 280, '23': 250, '30': 110, '27': 245, '21': 235, '7': 305, '28': 115, '4': 245, '10': 135, '9': 325, '17': 290, '31': 275, '32': 320, '20': 280, '35': 125, '18': 260, '34': 300, '19': 245, '33': 260, '16': 260, '22': 270, '0': 310, '24': 305, '5': 295, '8': 260, '3': 290, '6': 285, '25': 260, '13': 230, '1': 280, '29': 115, '2': 250, '26': 260, '12': 240}
#idx = [x[0] for x in sorted(geant_all_no_spt.items(), key=lambda x: x[1])]

plt.plot([geant_all_no_spt[key] if key in geant_all_no_spt else 0 for key in idx], 'b.-', label='All delays')
plt.plot([geant_no_delay[key] for key in idx], 'x-', label='No bird, link delays')
plt.ylabel('Convergence Time [ms]')
plt.xlabel('Link Failures')
plt.title('Sorted convergence time for GEANT links')
axes = plt.gca()
axes.set_ylim([0,550])
plt.legend()
plt.savefig('plot1.png', dpi=600)
plt.clf()



