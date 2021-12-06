# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as mtick

a = [1228.3,3.38,63.8,0.07,0.16,6.74,1896.18]  #数据
b = [0.12,-12.44,1.82,16.67,6.67,-6.52,4.04]
l = [i for i in range(7)]

fmt='%.2f%%'
yticks = mtick.FormatStrFormatter(fmt)  #设置百分比形式的坐标轴
lx=['t1','t2','t3','t4','t5','t6','t7']

fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.plot(l, b,'or-',label='rate');
ax1.yaxis.set_major_formatter(yticks)
for i,(_x,_y) in enumerate(zip(l,b)):
    plt.text(_x,_y,b[i],color='black',fontsize=10,)  #将数值显示在图形上
ax1.legend(loc=1)
ax1.set_ylim([-20, 30]);
ax1.set_ylabel('rate');

ax2 = ax1.twinx() # this is the important function
plt.bar(l,a,alpha=0.3,color='blue',label='production')
ax2.legend(loc=2)
ax2.set_ylim([0, 2500])  #设置y轴取值范围
plt.legend(loc="upper left")
plt.xticks(l,lx)
plt.tight_layout()
plt.show()
