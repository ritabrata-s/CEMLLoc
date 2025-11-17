'''
Convert image data from a root 3D histogram into numpy array.
The image data consists projected count distribution due to
energy distributions in the crystals.

The true theta phi angle value of the source location are 
added as the truth values.
'''

import ROOT
import sys
import numpy as np
import matplotlib.pyplot as plt

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# USAGE

print("~~~ Convert ROOT histgram into numpy array ~~~")

if len(sys.argv) != 2:
    print(" USAGE : % s < input file > " %( sys.argv[0]))
    sys.exit(1)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# I/O info
outFile = "datasrc-set1"

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Input data file (for histograms)

inFileName = sys.argv[1]
print(" Reading from ", inFileName)
inFile = ROOT.TFile.Open(inFileName, "READ")

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Source position information

# srcinfo = np.genfromtxt("/home/ritabrata/Work/CrystalEye/Prod/Localization/V2R8/srcpos-sam.txt", dtype=('U15', 'f', 'f'), delimiter=' ')
#srcinfo[0][0] = "{}{}".format(srcinfo[0][0], ".root")
#print(srcinfo[0][0])

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# List container of output

lst = []

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Get the histograms in the input file
hKeys = inFile.GetListOfKeys()
iter = hKeys.MakeIterator()
key = iter.Next()
histos = []
td = None
while key:
    if key.GetClassName() == "TH3F":
        td = key.ReadObj()
        hName = td.GetName()
        #print("found directory ", hName)
        histos.append(td)
    elif key.GetClassName() == "TNamed":
        td = key.ReadObj()
        posInfoData = td.GetTitle()
#         print("found file ", posInfoData)
    else:
        print("No histogram was found...")
    key = iter.Next()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Source position information

srcinfo = np.genfromtxt(posInfoData, dtype=('U15', 'f', 'f'), delimiter=' ')

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Loop on the histpgrams
#hist = inFile.Get("hPix99")

for hist in histos :
    print("Processing histogram: ", hist.GetName())

    title = hist.GetTitle()
    if title == "": # background
        truth = np.array([0, 0], dtype='f')
    else : # source
        title = title.strip(".root")
        print("title = ", title)

        # Find theta/phi information
        for x in srcinfo :
            if x[0] == title : 
                truth = np.array([x[1], x[2]], dtype='f')
                #print(x[1], x[2])

    # Get the image array
    content = np.zeros((hist.GetNbinsX(), hist.GetNbinsY(), hist.GetNbinsZ(), 1), dtype='f')
    for zbin in range(1, hist.GetNbinsZ() + 1):
        for ybin in range(1, hist.GetNbinsY() + 1):
            for xbin in range(1, hist.GetNbinsX() + 1):
                content[xbin - 1, ybin - 1, zbin - 1, 0] = hist.GetBinContent(xbin, ybin, zbin)

    lst.append(np.array([content, truth], dtype="object"))
    
#print(content.shape)

# for x in content:
#     print(x)

# plt.scatter(X, Y, Z, c=content, cmap=plt.hot())
# plt.show()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Save data

np.savez_compressed(outFile, arr=np.asarray(lst))

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
