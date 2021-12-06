# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np


fontSize = 11
plt.figure(figsize=(7, 4))
sns.set_style("whitegrid", {"axes.facecolor": "#ffffff"})
df = pd.read_csv('result/compare-time/ERTimeRaw.csv')
data = np.array(df[30:]['time ratio'])
tab = np.array(['Ours, L=1\n(CPM method)', 'Ours, L=2', 'Ours, L=4', 'Ours, L=$+\infty$', "Pure Monte Carlo"])
ax = sns.barplot(tab, data, saturation=0.5)
for p in ax.patches:
    height = p.get_height()
    ax.text(x=p.get_x() + p.get_width()/2,
            y=height,
            s='{:.2f}%'.format(height*100),
            ha='center',
            fontsize=fontSize)

# plt.tight_layout()
plt.xlabel('Method')
plt.ylabel('Normalized runtime')
plt.yscale('symlog', linthreshy=0.01)
plt.show()
