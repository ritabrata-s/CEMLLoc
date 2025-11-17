import tensorflow as tf
from tensorflow import keras
import sys
import numpy as np
import math
import tensorflow.keras.backend as K
import matplotlib.pyplot as plt

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I/O info
'''
modelFile = 'model33-set1.keras'
outFile = 'angdata33-set1'

def load_numpy_data_multifile(filelist):
    allindata  = []
    for fname in filelist:
        try:
            item = np.load(
                    fname,
                    allow_pickle=True,
                    encoding="bytes"
            )['arr']  # all data
            if not item.shape[0]: print ("[load_numpy_data_multifile] empy file, skipping: ", fname)
            else: allindata.append(item)
        except Exception as e:
            print ("[load_numpy_data_multifile] failed to load file: ", fname)
            print ("[load_numpy_data_multifile]  ... the error: ")
            print (e)
            print ("[load_numpy_data_multifile]  ... skipping this file.")
    data = np.concatenate(allindata) if allindata else np.array(allindata)
    
    return data

def prepare_data(data, verbose=False):
    #
    # The function prepares the data in the proper-shape numpy arrays
    #
    #   data - normally, it is a numpy array of the data for training or prediction
    #
    # Data structure:
    #   [:,0] - pixel images
    #   [:,1] - truth 2 variables - normally variables that are targeted at the regression optimisation, say src_th,src_ph

    caloimages    = data[:,0]
    truthdata     = data[:,1]

    # get tensor-like shape of the arrays
    caloimages    = caloimages.tolist()
    truthdata     = truthdata.tolist()

    caloimages    = np.array(caloimages, dtype='float32') # get a 3-dimansional array, 1st dimension - events, 2,3rd dimensions - image dimensions
    truthdata     = np.array(truthdata)                   #

    # all done
    return {
        'caloimages'    : caloimages,
        'truthdata'     : truthdata,
    }

def get_input_data():
    if len(sys.argv) < 2:
        print(" USAGE : % s < input file > " %( sys.argv[0]))
        sys.exit(1)

    # get all input data
    args = sys.argv[1:]
    np.random.seed(1234)
    np.random.shuffle(args)
    data = load_numpy_data_multifile(args)

    # randomly shuffle the sample
    np.random.shuffle(data)
    
    # input data size info
    print ("data.shape =       ", data.shape)

    # 'prepare' input data (in particular, pixel image, etc.)
    dt_      = prepare_data(data)
    dt_in    = dt_['caloimages']
    dt_truth = dt_['truthdata']

    return dt_in, dt_truth

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Calculate angle between the true and predicted location (vectors) for customized loss.

'''

@keras.utils.register_keras_serializable(package="my_loss", name="angle_uncer")
def angle_uncer(y_true, y_pred):
    # true vector
    vt_0 = keras.ops.sin(math.pi*y_true[:,0]/180.) * keras.ops.cos(math.pi*y_true[:,1]/180.)
    vt_1 = keras.ops.sin(math.pi*y_true[:,0]/180.) * keras.ops.sin(math.pi*y_true[:,1]/180.)
    vt_2 = keras.ops.cos(math.pi*y_true[:,0]/180.)
    vt = keras.ops.stack((vt_0, vt_1, vt_2), axis=1)
    
    # prediction vector
    vp_0 = keras.ops.sin(math.pi*y_pred[:,0]/180.) * keras.ops.cos(math.pi*y_pred[:,1]/180.)
    vp_1 = keras.ops.sin(math.pi*y_pred[:,0]/180.) * keras.ops.sin(math.pi*y_pred[:,1]/180.)
    vp_2 = keras.ops.cos(math.pi*y_pred[:,0]/180.)
    vp = keras.ops.stack((vp_0, vp_1, vp_2), axis=1)

    vtp = vt_0*vp_0 + vt_1*vp_1 + vt_2*vp_2
    vtt = vt_0*vt_0 + vt_1*vt_1 + vt_2*vt_2
    vpp = vp_0*vp_0 + vp_1*vp_1 + vp_2*vp_2

    err_ang = keras.ops.arccos(vtp / (keras.ops.sqrt(vtt * vpp))) * 180 / math.pi

    return keras.ops.mean(err_ang)

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Calculate angle between the true and predicted location (vectors).

'''

def calc_angle(y_true, y_pred):
    # true vector
    vt_0 = keras.ops.sin(math.pi*y_true[:,0]/180.) * keras.ops.cos(math.pi*y_true[:,1]/180.)
    vt_1 = keras.ops.sin(math.pi*y_true[:,0]/180.) * keras.ops.sin(math.pi*y_true[:,1]/180.)
    vt_2 = keras.ops.cos(math.pi*y_true[:,0]/180.)
    vt = keras.ops.stack((vt_0, vt_1, vt_2), axis=1)
    
    # prediction vector
    vp_0 = keras.ops.sin(math.pi*y_pred[:,0]/180.) * keras.ops.cos(math.pi*y_pred[:,1]/180.)
    vp_1 = keras.ops.sin(math.pi*y_pred[:,0]/180.) * keras.ops.sin(math.pi*y_pred[:,1]/180.)
    vp_2 = keras.ops.cos(math.pi*y_pred[:,0]/180.)
    vp = keras.ops.stack((vp_0, vp_1, vp_2), axis=1)

    vtp = vt_0*vp_0 + vt_1*vp_1 + vt_2*vp_2
    vtt = vt_0*vt_0 + vt_1*vt_1 + vt_2*vt_2
    vpp = vp_0*vp_0 + vp_1*vp_1 + vp_2*vp_2

    err_ang = keras.ops.arccos(vtp / (keras.ops.sqrt(vtt * vpp))) * 180 / math.pi

    return err_ang

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Main run program.

'''

def run():
    # Load the trained model
    model = keras.models.load_model(modelFile)

    # Get data
    dt_in, dt_truth = get_input_data()

    # Predict
    prediction = model.predict(dt_in, verbose=1)

    ang = calc_angle(dt_truth, prediction)
    ang = K.eval(ang)
    np.savez_compressed(outFile, ang=np.asarray(ang))

    '''
    fig, ax = plt.subplots(figsize=[12,9])
    # create histogram within output
    N, bins, patches = ax.hist(ang, bins=1800, range=(0, 180), color="#777777") #initial color of all bins
    
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
    
    plt.show()
    '''

if __name__ == "__main__":
    run()
