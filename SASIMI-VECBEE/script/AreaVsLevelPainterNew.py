# -*- coding: utf-8 -*-


import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np


fontSize = 10
plt.figure(figsize=(8.5, 4))
sns.set_style("whitegrid", {"axes.facecolor": "#ffffff"})
df = pd.read_excel('result/maxLevel-NMED/CompareAreaDelay.xlsx')
print(df)

ax = sns.barplot(x='Circuit', y='Normalized area', hue='Method', data=df, saturation=0.4)
for p in ax.patches:
    height = p.get_height()
    ax.text(x=p.get_x() + p.get_width()/2,
            y=height,
            s='{:.1f}%'.format(height*100),
            ha='center',
            fontsize=fontSize)

# plt.tight_layout()
plt.show()
