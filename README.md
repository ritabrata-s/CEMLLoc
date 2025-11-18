# CEMLLoc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Machine Learning algorithm for Crystal Eye transient source localization.

Three different schemes for the localization with three different input data information.

1DNeuralNet: Using 1D pixel Id. vs. normalized counts in the pixels for the source (and background).
2DNeuralNet: Using 2D image of the customized projection in the detector XY plane of the normalized counts in the pixels.
3DNeuralNet: Using 3D voxel array of the detector geometry with normalized counts in each crystals.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1DNeuralNet:
-----------------------------

2DNeuralNet:
-----------------------------
Generation of 2D projected image of the normalized counts in the detector pixels (for sources at different location; used for training and test):
    Source code: genSrc.C (root macro)
        Inputs: Background contributions (Normalized histogram data from CEAna) -- should modify in the source code
            Source contributions (Normalized histogram data from CEAna)
            Position information of the sources (theta/phi location in sky)
            Image pixel to crystal Id. conversion map information
            -- should modify in the source code
        Output: 2D histogram of the detector image

Convert 2D projected image of the normalized counts in the detector pixels to numPy array:
    Source code: convertTh2ToArr.py (python script)
        Input: 2D histogram of the detector image (from previous step) -- should be given as argument of the execution
        Output: numPy array in .npz file -- change the file name in the source code

Train the CNN model with numPy array data:
    Source code: trainLoc.py (python script)
        Input: numPy array data file(s) -- should be given as argument of the execution
        *Change input data shape according to the image resolution in the source code
        Output: trained model and training history -- change the model and history file names in the source code

Calculate or predict the location of sources using another data set:
    Source code: predict.py (python script)
        Input: numPy array data file for set of sources -- should be given as argument of the execution
           Trained model from the previous step -- change the model file in the source code
        Output: Angle between reconstructed and true location data -- change the file name in the source code

Draw the angular deviation histogram and calculate 68% and 95% containment radii:
    Source code: plotang.py (python script)
        Input: numPy array data file from prediction step -- should be given as argument of the execution
        Output: result pdf -- change the file name in the source code

3DNeuralNet:
-----------------------------
