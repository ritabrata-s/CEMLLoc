# 
# Usage:
#   python visualize.py history.json
#

import matplotlib.pyplot as plt
import json
import sys

with open(sys.argv[1],'r') as fin:
    datain  = fin.read()
    history = json.loads(datain)

# Plot training & validation loss values
#plt.plot(history['loss'])
#plt.plot(history['val_loss'])
plt.plot(history['mean_absolute_error'])
plt.plot(history['val_mean_absolute_error'])
plt.title('Model loss')
plt.ylabel('Loss')
plt.xlabel('Epoch')
plt.yscale('log')
plt.ylim((1,10000000))
plt.legend(['Train', 'Test'], loc='upper left')
plt.show()
