# -*- coding: utf-8 -*-


import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np


fontSize = 11
plt.figure(figsize=(7, 4))
sns.set_style("whitegrid", {"axes.facecolor": "#ffffff"})
df = pd.read_excel('result/AERD/aerd_new.xlsx')
print(df)

ax = sns.barplot(x='Method', y='Error', hue='Error type', data=df, saturation=0.4)
for p in ax.patches:
    height = p.get_height()
    if height == 0:
        ax.text(x=p.get_x() + p.get_width()/2,
                y=height,
                s='{:.0f}%'.format(height*100),
                ha='center',
                fontsize=fontSize)
    else:
        ax.text(x=p.get_x() + p.get_width()/2,
                y=height,
                s='{:.2f}%'.format(height*100),
                ha='center',
                fontsize=fontSize)

plt.yscale('symlog', linthreshy=0.001)
# plt.tight_layout()
plt.show()
