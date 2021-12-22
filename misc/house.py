import matplotlib.pyplot as plt

# all but failure detected at the same time on both routers
#house_all = {'4': 115, '0': 230, '5': 230, '3': 220, '1': 230, '2': 230}
#house_no_delay = {'4': 110, '0': 110, '5': 110, '3': 110, '1': 110, '2': 110}
house_all = {'4': 115, '0': 230, '5': 230, '3': 220, '1': 230, '2': 230}
house_no_delay = {'4': 110, '0': 110, '5': 110, '3': 110, '1': 110, '2': 110}

idx = [x[0] for x in sorted(house_no_delay.items(), key=lambda x: x[1])]

plt.plot([house_all[key] for key in idx], 'b.-', label='All delays')
plt.plot([house_no_delay[key] for key in idx], 'x-', label='No bird, link delays')
plt.ylabel('Convergence Time [ms]')
plt.xlabel('Link Failures')
plt.title('Sorted convergence time for HOUSE links')
axes = plt.gca()
axes.set_ylim([0,550])
plt.legend()
plt.savefig('house.png', dpi=600)
plt.clf()



