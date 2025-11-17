import sys
import numpy as np
import matplotlib.pyplot as plt

# I/O
outFile = 'angDev33-set1.pdf'

# Get data
args = sys.argv[1:]
for fname in args:
    ang = np.load(fname)['ang']
#print(ang)

# Plot angular deviation
fig, ax = plt.subplots(figsize=[12,9])

# create histogram within output
N, bins, patches = ax.hist(ang, bins=1800, range=(0, 180), color="#777777")

# Calculate ruuning integartion
xarr = bins[:-1].copy()
sarr = N.copy()
tot = sum(sarr)
sumed = 0
ind = 0
for val in N:
    sumed = sumed + val
    sarr[ind] = sumed*100/tot
    ind += 1
ax.plot(xarr, sarr)

# 68 and 95% C.L.
ind = np.where(sarr >= 68)[0][0]
print(ind)
print("%.1f, %.1f" % (xarr[ind-1], sarr[ind-1]))
print("%.1f, %.1f" % (xarr[ind], sarr[ind]))
plt.axvline(x=xarr[ind], color='gray', linestyle='dashed')
plt.text(xarr[ind]+0.1, 67, f'68% C.L. {xarr[ind]:0.1f} (deg)', size=20)
ind = np.where(sarr >= 95)[0][0]
print(ind)
print("%.1f, %.1f" % (xarr[ind-1], sarr[ind-1]))
print("%.1f, %.1f" % (xarr[ind], sarr[ind]))
plt.axvline(x=xarr[ind], color='gray', linestyle='dashed')
plt.text(xarr[ind]+0.1, 91, f'95% C.L. {xarr[ind]:0.1f} (deg)', size=20)

plt.xlim(0,20)
plt.ylim(0,100)
plt.xlabel("Angle (degree)", loc='right', size=20)
plt.ylabel("No. of events/C.L. (%)", loc='top', size=20)
plt.xticks(fontsize=20)
plt.yticks(fontsize=20)

plt.savefig(outFile, dpi=200, bbox_inches='tight')

plt.show()

