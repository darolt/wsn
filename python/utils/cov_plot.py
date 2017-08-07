"""
========
Barchart
========

A bar plot with errorbars and height labels on individual bars
"""
import numpy as np
import matplotlib.pyplot as plt

plt.rcParams.update({'font.size': 14})

N = 3
cov_means = (0.6509, 0.8648, 0.9445)
cov_std   = (0.0637, 0.0451, 0.0206)

ind = np.arange(N)  # the x locations for the groups
width = 0.20       # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(ind + 0*width, cov_means, width, color='r', yerr=cov_std)

over_means = (0.4486, 0.3238, 0.3238)
over_std   = (0.2113, 0.0980, 0.0982)
rects2 = ax.bar(ind + 1*width, over_means, width, color='b', yerr=over_std)

sleep_means = (0.3572, 0.2949, 0.1801)
sleep_std   = (0.0661, 0.0467, 0.033)
rects3 = ax.bar(ind + 2*width, sleep_means, width, color='g', yerr=sleep_std)

alive_means = (1.09257, 1, 0.8695)
rects4 = ax.bar(ind + 3*width, alive_means, width, color='y')

# add some text for labels, title and axes ticks
ax.set_ylabel('Rates')
ax.set_xticks(ind + 1.5*width)
ax.set_xticklabels((r'$\alpha=1, \beta=0$', r'$\alpha=0.5, \beta=0.5$', r'$\alpha=0, \beta=1$'))

ax.legend((rects1[0], rects2[0], rects3[0], rects4[0]), ('Coverage rate', 'Overlapping rate', 'Sleeping rate', 'Lifetime proportion'), loc='upper center', bbox_to_anchor=(0.5, -0.1),
          fancybox=True, shadow=False, ncol=2)

def autolabel(rects):
    """
    Attach a text label above each bar displaying its height
    """
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                '%d' % int(height),
                ha='center', va='bottom')

#autolabel(rects1)
#autolabel(rects2)
#autolabel(rects3)
#autolabel(rects4)

plt.show()
