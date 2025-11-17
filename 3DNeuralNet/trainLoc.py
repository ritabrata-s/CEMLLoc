'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create and train a model for transient source localization, based on the image
of count distributions in the pixels of the detector.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
'''
# import os
# os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'
import tensorflow as tf
from tensorflow import keras
import tensorflow.keras.backend as K
import sys
import numpy as np
import json
import math

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hardware specific settings
'''

# fixing GPU RTX 2080 specific problem on Andrii's desktop
# physical_devices = tf.config.list_physical_devices('GPU')           # GPU RTX 2080 issue
# print(physical_devices)
# tf.config.experimental.set_memory_growth(physical_devices[0], True) # GPU RTX 2080 issue 
# tf.config.set_visible_devices([], 'GPU')


'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I/O info
'''
historyFile = 'history-set1.json'
modelFile = 'model-set1.keras'
inpShapeXY = 50
inpShapeZ = 25

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Calculate angle between the true and predicted location (vectors).

'''

#@keras.utils.register_keras_serializable(package="my_loss", name="angle_uncer")
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
    #err_ang = keras.ops.arccos((np.vecdot(vt,vp)) / (keras.ops.sqrt(np.vecdot(vt,vt) * np.vecdot(vp,vp))))
    #err_ang = keras.ops.sort(err_ang)
    #print(err_ang*180/math.pi)

    #ind = np.int8(np.floor(keras.ops.shape(err_ang)[0]))
    #print(ind)
    #cl68 = err_ang[ind-1]
    #print(cl68)

    #return cl68

    return keras.ops.mean(err_ang)
    #return err_ang

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Calculate 68% c.l. value of the angular errors.


def angle_uncer_loss(y_true, y_pred):
    err_ang = angle_uncer(y_true, y_pred)

    #cl68 = err_ang[np.floor(0.68*err_ang.size).astype(np.uint8)]
    #print(cl68)

    return keras.ops.mean(err_ang)
'''

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Calculate 68% c.l. value of the angular errors.


def angle_uncer_mean(y_true, y_pred):
    err_ang = keras.ops.sort(angle_uncer(y_true, y_pred))
    #ind = np.int8(np.floor(keras.ops.shape(err_ang)[0]))
    #print(ind)
    #cl68 = err_ang[ind-1]
    #print(cl68)

    #cl68 = err_ang[np.floor(0.68*err_ang.size).astype(np.uint8)]
    #print(cl68)

    return keras.ops.mean(err_ang)
'''

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Prepare and compile the CNN model

'''

def get_convnet_calo_model(out_shape=2, dropout=None):
    # inputs
    #inputCalo    = keras.Input(shape=(200, 200, 1))
    inputCalo    = keras.Input(shape=(inpShapeXY, inpShapeXY, inpShapeZ, 1))

    # Calo model image
    caloconlayer    = keras.layers.Conv3D(128, (4,4,4), activation="relu")(inputCalo)
#     caloconlayer    = keras.layers.MaxPooling3D((2,2,2))(caloconlayer)
    caloconlayer    = keras.layers.Dropout(dropout)(caloconlayer) if dropout else caloconlayer
    caloconlayer    = keras.layers.Conv3D(64, (4,4,4), activation="relu")(caloconlayer)
#     caloconlayer    = keras.layers.MaxPooling3D((2,2,2))(caloconlayer)
    caloconlayer    = keras.layers.Dropout(dropout)(caloconlayer) if dropout else caloconlayer
    caloconlayer    = keras.layers.Conv3D(32, (4,4,4), activation="relu")(caloconlayer)
    caloconlayer    = keras.layers.MaxPooling3D((2,2,2))(caloconlayer)
    caloconlayer    = keras.layers.Dropout(dropout)(caloconlayer) if dropout else caloconlayer
    
#     print(caloconlayer.shape[1],caloconlayer.shape[2],caloconlayer.shape[3])
    
    # ... full-size filters of conv net
    caloconlayer    = keras.layers.Conv3D(100, (caloconlayer.shape[1],caloconlayer.shape[2],caloconlayer.shape[3]), activation="relu")(caloconlayer)
    caloconlayer    = keras.layers.Dropout(dropout)(caloconlayer) if dropout else caloconlayer
    caloconlayer    = keras.layers.Flatten()(caloconlayer)

    # Calo model conclude to output shape
    caloconlayer = keras.layers.Dense(50, activation="relu")(caloconlayer)
    caloconlayer = keras.layers.Dropout(dropout)(caloconlayer) if dropout else caloconlayer
    caloconlayer = keras.layers.Dense(out_shape, activation="linear")(caloconlayer)
    #print(caloconlayer.shape)
    calo = keras.Model(inputs=inputCalo, outputs=caloconlayer)

    # Compile the model
    #calo.compile(loss=angle_uncer, optimizer='adam', metrics=[angle_uncer])
    calo.compile(loss='mean_absolute_error', optimizer='adam', metrics=['mean_absolute_error','mean_squared_error'])

    # Model output shape 
    print("Calo convnet model prepared, input/output dimension:", calo.input_shape, calo.output_shape)

    # Return the model
    return calo

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Prepare the data:

    The function prepares the data in the proper-shape numpy arrays
    data - normally, it is a numpy array of the data for training or prediction

    Data structure:
      [:,0] - pixel images
      [:,1] - truth 2 variables - normally variables that are targeted at the
              regression optimisation, say src_th,src_ph

'''

def prepare_data(data):
    caloimages    = data[:,0]
    truthdata     = data[:,1]

    # get tensor-like shape of the arrays
    caloimages    = caloimages.tolist()
    truthdata     = truthdata.tolist()

    # get a 3-dimansional array, 1st dimension - events, 2,3rd dimensions - image dimensions
    caloimages    = np.array(caloimages, dtype='float32') 
    truthdata     = np.array(truthdata)                     # get a 1-d array, two variables

    # all done
    return {
        'caloimages' : caloimages, 
        'truthdata'  : truthdata, 
    }

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Load the data from files

'''

def load_numpy_data_multifile(filelist):
    allindata  = []
    
    for fname in filelist:
        try:
            item = np.load(
                    fname,
                    allow_pickle=True,
                    encoding="bytes"
            )['arr']  # all data
            if not item.shape[0]: 
                print("[load_numpy_data_multifile] empty file, skipping: ", fname)
            else: 
                allindata.append(item)
        except Exception as e:
            print("[load_numpy_data_multifile] failed to load file: ", fname)
            print("[load_numpy_data_multifile]  ... the error: ")
            print(e)
            print("[load_numpy_data_multifile]  ... skipping this file.")
            
    data = np.concatenate(allindata) if allindata else np.array(allindata)
    
    return data

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Get input data --- from a list of files given as the argument of the main
function.

'''

def get_input_data():
    # get all input data
    args = sys.argv[1:]
    np.random.seed(1234)
    np.random.shuffle(args)
    data = load_numpy_data_multifile(args)
    
    # randomly shuffle the sample
    np.random.shuffle(data)
    
    # split data into training and validation
    val_ratio  = 0.1
    val_ratio  = int(data.shape[0] * val_ratio)
    data_train = data[:-val_ratio]
    data_val   = data[-val_ratio:]
    
    # input data size info
    print("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    print("data.shape =       ", data.shape)
    print("data_train.shape = ", data_train.shape)
    print("data_val.shape   = ", data_val.shape)
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n")
    
    # 'prepare' input data (in particular, pixel image, etc.)
    dt_      = prepare_data(data_train)
    dv_      = prepare_data(data_val)
    dt_truth = dt_['truthdata'] 
    dv_truth = dv_['truthdata'] 
    dt_in    = dt_['caloimages']
    dv_in    = dv_['caloimages']
    
    return dt_in, dt_truth, dv_in, dv_truth

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The main function to run the code

To get the data, call the CNN model, adjust learning rate, train the model,
save the model.

'''

def run():
    # USAGE
    print("\n~~~ Training NN model with 3D data ~~~")
    
    if len(sys.argv) != 2:
        print("\nUSAGE : % s < input file > \n" %( sys.argv[0]))
        sys.exit(1)

    # Get data
    print("\n\nRetrieving the data...")
    dt_in, dt_truth, dv_in, dv_truth = get_input_data()

#     d_test = dt_in[0]
#     print("test shape: ", dt_in.shape)

    # Loss/metric function
    #d_t = K.constant(np.array([[80, 0], [50, 0], [70, 0]]))
    #d_p = K.constant(np.array([[80, 181], [70, 0], [50, 0]]))
    #angle_error(d_t, d_p)
    #angle_uncer(dv_truth, dv_truth)
    
    # Get the ML model
    print("\nPreparing the CNN model...")
    model = get_convnet_calo_model()

    # adjust learning rate
    print("\nAdjusting learning rate...")
    reduce_lr = keras.callbacks.ReduceLROnPlateau(
        monitor='val_loss', factor=0.5, patience=4, verbose=1, mode='auto',
        min_delta=0.0001, cooldown=0, min_lr=0.0000001,
    )

    # fit
    print("\nTraining the model...")
    history = model.fit(dt_in, dt_truth, validation_data=(dv_in, dv_truth),
                        epochs=100, verbose=2, callbacks=[reduce_lr]) # step 1
    print(history.history.keys())

    # save trainig history
    with open(historyFile, 'w') as fh:
        # del history.history['learning_rate'] # for some reason it can not be serialised by json
        json.dump(history.history, fh)
        
    # save the model
    # model.save('model.h5')
    model.save(modelFile)
    
    #d_test = dt_in[0]
    #print("test shape: ", d_test.shape)
    #model.predict(dt_in)
'''
'''

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Declare main function

python train.py filelist

'''

if __name__ == "__main__":
    run()

'''
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
'''
   
