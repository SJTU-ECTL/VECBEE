from scipy import stats
import numpy as np

def HT(k, n, p0, CL = 0.05):
    pValue = stats.binom.cdf(k=k, n=n, p=p0)
    # print(k, n, p0, k / n)
    # print("p-value =", pValue)
    if pValue <= CL:
        pass
        # print("reject H0, accept H1")
    else:
        print(k, n, p0, k / n)
        print("p-value =", pValue)
        print("no evidence to reject H0")
    # assert pValue <= CL

M = 100000
# estimatedER = np.array([0.05, 0.03679, 0.04643, 0.04975, 0, 0, 0.04963, 0.04981, 0.04854, 0.04098, 0.04813, 0.04718])
estimatedER = np.array([0.05, 0.03679, 0.04942, 0.04975, 0, 0, 0.04963, 0.04981, 0.04854, 0.04098, 0.04813, 0.04718])
erBound = 0.05
for i in range(len(estimatedER)):
    HT(estimatedER[i] * M, M, erBound)
# HT(80, 10000, 0.01)