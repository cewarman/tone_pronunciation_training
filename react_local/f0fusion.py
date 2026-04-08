import numpy as np
import sox
import pysptk
import math
from scipy.cluster.vq import vq, kmeans, whiten
from argparse import ArgumentParser
import os, sys

BANNER = r"""
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@&&@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(/////(&@%/////////&@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@/////////##@@#//////&@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@&////#&@@@(@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@*     %@@@@@@@@@%%@@@@@@@@@&,.  ,%@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@          &@@@@@@@%#@@@@@@@(         /@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@&                                      @@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@*                                     @@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@&&.                               .@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@.                                &@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@                                   @@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@/                                    @@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@            %*        *&             #@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@/                .@@@@                 @@@@@@@@@@@@@@
@@@#*/, , (@@@@@@@@@@@                   /                   &@@@@@@@@@@@@@
@# * (@@@&  %@@@@@@@@@                   /*                  %@@@@@@@@@@@@@
@%  @@@.      #@@@@@@@                                       .@@@@@@@@@@@@@
@@@              @@@@@,                                       #@@@@@@@@@@@@
@@@@                ,(@@@@,                               @@@@@@&@@@@@@@@@@
@@@@@                  (@@@@@@(                      (@@@@@@@@    @@@@@@@@@
@@@@@@                   *@@@@@@@@#              &@@@@@@@@@&       .@@@@@@@
@@@@@@(                     @@@@@@@@@@%     *@@@@@@@@@@@@.           (@@@@@
@@@@@@@                       (@@@@@@@@@@@@@@@@@@@@@@@,                @@@@
@@@@@@@@                         &@@@@@@@@@@@@@@@@@,                    %@@
@@@@@@@@%                           %@@@@@@@@@@#                         (@ 
"""

  
 

def main(inWavFilename, L, H, p, o):
    
    lowerf0 = L
    upperf0 = H
    
    inFn = inWavFilename
    #outFileName = options.OutF0Filename


    # default settings
    fs_for_rapt = 16000
    fNum = 5
    max_f0_octave = 4 # unit: octave, i.e., 2^max_f0_octave Hz


    # frame interval in sample
    psmp = 80
    if p==5:
        psmp = 80
    elif p==10:
        psmp = 160
    else:
        psmp = 80 
        

    # create transformer
    tfm = sox.Transformer()

    # read waveform
    x_ori = tfm.build_array(inFn)
    #print('x_ori\'s shape:', np.shape(x_ori))
    fs = sox.file_info.sample_rate(inFn)
    wlen = len(x_ori)
    fsftsmp_5 = fs*0.005; 
    fsftsmp_10 = fs*0.01; 
    #print('len=', wlen, 'smp rate=', fs)


    nFrm5ms = math.ceil(wlen/fsftsmp_5)
    nFrm10ms = math.ceil(wlen/fsftsmp_10)

    #print('target frame # for 5ms frame shift=', nFrm5ms)
    #print('target frame # for 10ms frame shift=', nFrm10ms)


    # convert waveform's sample rate for RAPT
    tfm.set_output_format(file_type='wav', rate=fs_for_rapt, bits=16, channels=1, encoding='signed-integer')
    x = tfm.build_array(input_filepath=inFn)
    #print('x\'s shape:', np.shape(x))


    # F0 extraction by RAPT
    raw_f0data = []
    lens = []
    upSamplingRatios = []

    raw_f0data.append(pysptk.sptk.rapt(x, fs_for_rapt, 160, min=lowerf0, max=upperf0, voice_bias=0.5, otype='f0'))
    lens.append(len(raw_f0data[0])*2)
    upSamplingRatios.append(2)

    raw_f0data.append(pysptk.sptk.rapt(x, fs_for_rapt, 80, min=lowerf0, max=upperf0, voice_bias=0.3, otype='f0'))
    lens.append(len(raw_f0data[1]))
    upSamplingRatios.append(1)

    raw_f0data.append(pysptk.sptk.rapt(x, fs_for_rapt, 160, min=lowerf0, max=upperf0, voice_bias=0.3, otype='f0'))
    lens.append(len(raw_f0data[2])*2)
    upSamplingRatios.append(2)

    raw_f0data.append(pysptk.sptk.rapt(x, fs_for_rapt, 80, min=60, max=400, voice_bias=0.3, otype='f0'))
    lens.append(len(raw_f0data[3]))
    upSamplingRatios.append(1)

    raw_f0data.append(pysptk.sptk.rapt(x, fs_for_rapt, 160, min=60, max=400, voice_bias=0.3, otype='f0'))
    lens.append(len(raw_f0data[4])*2)
    upSamplingRatios.append(2)




    max_len = max(lens)
    #print(max_len)

    f0data = np.empty((max_len, fNum))

    # merge all extracted into one 2d array
    for i in range(0, fNum):
        f = raw_f0data[i]   
        f = np.reshape(f,(f.size,1))
        #print(np.shape(f))
        upsampled_array = np.kron(f, np.ones((upSamplingRatios[i],1)))
        upsampled_array = np.transpose(upsampled_array, [1,0])
        #print('upsampled_array:', i, np.shape(upsampled_array))
        #print('lens :', lens[i])
        f0data[0:lens[i],i] = upsampled_array


    # voiced voting threshold
    voicedThreshold = math.floor(fNum/2) + 1

    # start to vote
    f0_result_5ms = np.zeros((max_len, 1))
    for n in range(0, max_len):
        #print('n=', n)
        data = f0data[n,:]
        data = np.transpose(data)
        result = np.where(data > 0.0)
        if len(result[0])>=voicedThreshold:
            #print('frame index=', n)
            data = data[result[0]]
            max_cluster_num = len(data)   


            for k in range(1, max_cluster_num+1):
                kmr = kmeans(data, k)
                code, dist = vq(data, kmr[0])
                pit = kmr[0][code]
                
                # The maximum F0 (pit) that is regarded as the same F0 value is designed
                # to be 2^max_f0_octave Hz (octave makes sense)
                pit = pit / (2**max_f0_octave) 
                
                #print('k=', k, 'cluster=', kmr)
                #print('code=', code, 'dist=', dist)
                f0_dist_check = np.where(np.less(dist, pit))
                #print('f0_dist_check', f0_dist_check[0])
                values, counts = np.unique(code, return_counts=True)
                #print('values:', values, 'counts:', counts)

                if len(f0_dist_check[0])==len(data):
                    values, counts = np.unique(code, return_counts=True)
                    #print('values:', values, 'counts:', counts)
                    cc = np.argmax(counts)
                    best_f0 = kmr[0][cc]
                    f0_result_5ms[n,0] = best_f0
                    #print('cc=', cc, 'best_f0', best_f0)
                    #input("Press enter to continue\n\n")
                    break   



    output_f0_5ms = np.concatenate((f0_result_5ms,f0data),axis=1)    
    if p==10:
        output_f0_10ms = output_f0_5ms[0:output_f0_5ms.size:2,:]

    #if p==5:
    #    print('generated frame # for 5ms frame shift=', np.shape(output_f0_5ms)[0])
#
    #if p==10:
    #    print('generated frame # for 10ms frame shift=', np.shape(output_f0_10ms)[0])

    if p==5:
        if np.shape(output_f0_5ms)[0] > nFrm5ms:
            output_f0_5ms = output_f0_5ms[0:nFrm5ms,:]
            #print('output_f0_5ms resize to length= ', np.shape(output_f0_5ms)[0])

        if np.shape(output_f0_5ms)[0] < nFrm5ms:
            output_f0_5ms = np.concatenate((output_f0_5ms,np.zeros(nFrm5ms-np.shape(output_f0_5ms)[0],np.shape(output_f0_5ms)[1])),axis=0)  
            #print('output_f0_5ms resize to length= ', np.shape(output_f0_5ms)[0])  

    if p==10:
        if np.shape(output_f0_10ms)[0] > nFrm10ms:
            output_f0_10ms = output_f0_10ms[0:nFrm10ms,:]
            #print('output_f0_10ms resize to length= ', np.shape(output_f0_10ms)[0])

        if np.shape(output_f0_10ms)[0] < nFrm10ms:
            output_f0_10ms = np.concatenate((output_f0_10ms,np.zeros(nFrm10ms-np.shape(output_f0_10ms)[0],np.shape(output_f0_10ms)[1])),axis=0)  
            #print('output_f0_10ms resize to length= ', np.shape(output_f0_10ms)[0])  

    if p==5:
        if o==2:
            np.seterr(all="ignore")
            output_f0_5ms = np.log(output_f0_5ms)
            idx = np.where(np.isinf(output_f0_5ms))
            output_f0_5ms[idx] = 0.0
        #np.savetxt(outFileName, output_f0_5ms, fmt='%.7f')
        output_f0_5ms = np.float32(output_f0_5ms[:,0])
        with os.fdopen(sys.stdout.fileno(), "wb", closefd=False) as stdout:
            stdout.write(output_f0_5ms)
            stdout.flush()

    if p==10:
        if o==2:
            np.seterr(all="ignore")
            output_f0_10ms = np.log(output_f0_10ms)
            idx = np.where(np.isinf(output_f0_10ms))
            output_f0_10ms[idx] = 0.0
        #np.savetxt(outFileName, output_f0_5ms, fmt='%.7f')
        output_f0_10ms = np.float32(output_f0_10ms[:,0])
        return output_f0_10ms


# only the main process will execute this part.
if __name__ == "__main__":
    main()