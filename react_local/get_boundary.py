import asyncio
import websockets
from IPython.display import clear_output
import os
from pathlib import Path
import phs2syl
import librosa
import numpy as np
#from f0fusion import main as getf0
import parselmouth
import tf_keras as k3
#import pickle

rec_model = k3.models.load_model('/home/cewarman/research/ASR/tone_rec_large/models/TAAN_fully_model/')
ppc_model = k3.models.load_model('/home/cewarman/research/ASR/PPC/models/MH_fully_PPC')
#predrest=rec_model.predict(np.array([np.ones((81,65))]),verbose=0)
#def get_GOP(res, t):
#    tv=res[t-1]
#    new_arr = np.delete(res, t-1)
#    #print(np.log(tv/np.max(new_arr)), tv, np.max(new_arr))
#    return np.log(tv/np.max(new_arr))
#with open("../../ASR/tone_rec_large/data/tmpdir/GOPdist.pkl", "rb") as f:
#    GOPdist = pickle.load(f)

psdict = phs2syl.get_dict()
#print(psdict.tm['ini_zn_FNULL2'])
def syllable_boundary_normalize(p):
    retval=""
    with open(p,'r',encoding='utf8') as f:
        rawsb=[line.strip().split() for line in f.readlines()]
    #print(rawsb)
    noraw=[]
    tmpstr=""
    for i in range(len(rawsb)):
        if(len(rawsb[i])==4):
            noraw.append(rawsb[i])
            if(noraw[len(noraw)-2][3]!='<SILENCE>'):
                #print(noraw)
                noraw[len(noraw)-2][3]=psdict.tm[tmpstr]
            tmpstr=""
        tmpstr=tmpstr+rawsb[i][2][:-2] if rawsb[i][2][:1]!='n' else tmpstr+rawsb[i][2][:-3]
    noraw[-1][1]=rawsb[-1][1]
    #print(noraw)
    for i in range(len(noraw)-1):
        noraw[i][1]=noraw[i+1][0]
    #print(noraw)
    for i in range(len(noraw)):
        if(noraw[i][-1]=='<SILENCE>'):
            retval=retval+noraw[i][0]+" "+noraw[i][1]+" ϵ\n"
        else:
            retval=retval+noraw[i][0]+" "+noraw[i][1]+" "+noraw[i][-1]+'\n'
    #print(retval)
    return retval
def phi(j,i,N):
    if j==0:
        basis = 1
    if j==1:
        if N<1:
            basis = 0
        else:
            basis = (12 * N / (N + 2))**(0.5) * (i/N-0.5)
    if j==2:
        if N<2:
            basis = 0
        else:
            basis =  (180 * N**3 / (N - 1) / (N + 2) / (N + 3))**0.5 * ( (i/N)**2 - i/N + (N-1)/(6*N))
    if j==3:
        N1 = N
        N2 = N**2
        N5 = N**5
        N3 = N**3
        I1 = i
        I2 = I1**2
        I3 = I1**3
        if N<3:
            basis = 0
        else:
            basis = ((2800*N5/(N1-1)/(N1-2)/(N1+2)/(N1+3)/(N1+4))**0.5) * ( I3/N3 - 3/2*I2/N2 + (6*N2-3*N1+2)/10/N3*I1 - (N1-1)*(N1-2)/20/N2)
    return basis
def Expansion_Pitch(n_smp,a):
    N = n_smp-1
    pitch=[]
    for i in range(N):
        pitch.append(a[0] * phi(0,i,N) + a[1] * phi(1,i,N) + a[2] * phi(2,i,N) + a[3] * phi(3,i,N))
    return pitch
def compute_ppc(feas, sylboundaries):
    predresp=ppc_model.predict(feas,verbose=0)
    #print(predresp)
    ppcs=[]
    for i in range(len(predresp[0])):
        ppcs.append(Expansion_Pitch(int(predresp[1][i]),predresp[0][i]))
    #print(ppcs)

    syltoks = [syl.split(' ') for syl in sylboundaries.split('\n')[:-1]]
    feas=[]
    #print(syltoks)
    f0=np.zeros(int(float(syltoks[-1][1])*100))
    ppcidx=0
    for i in range(len(syltoks)):
        if(syltoks[i][2] == 'ϵ'):
            continue
        else:
            st=int(float(syltoks[i][0])*100)
            et=int(float(syltoks[i][1])*100)
            ppc=ppcs[ppcidx]
            ppcst=st+round((et-st-len(ppc))*0.5)
            #print(st,et,len(ppc),ppcst)
            f0[ppcst:ppcst+len(ppc)]=ppc
            ppcidx=ppcidx+1
    return f0
            
def get_pitch(group, wave_fn, feas, sylboundaries, output_dir):
    #f0=getf0(wave_fn,60,400,10,1)
    
    #y, sr = librosa.load(wave_fn, sr=16000)
    #f0=librosa.yin(y, frame_length=2048, fmin=50, fmax=500, hop_length=160, sr=sr)
    #condition = f0 <= 50 
    #f0[condition] = 0
    #condition = f0 >= 500
    #f0[condition] = 0
    snd = parselmouth.Sound(wave_fn)
    pitch = snd.to_pitch_ac(time_step=0.01, pitch_floor=75, pitch_ceiling=500)
    #print(pitch, pitch.selected_array['frequency'],type(pitch.selected_array['frequency']))
    rpc=pitch.selected_array['frequency']
    rpc=np.log(rpc+1)
    with open('{}/{}.pc'.format(output_dir,wave_fn.split('/')[-1].split('.')[0]),'w') as f:
        for i in range(len(rpc)):
            f.write("{}\n".format(rpc[i]))

    ppc=compute_ppc(feas, sylboundaries)
    with open('{}/{}.ppc'.format(output_dir,wave_fn.split('/')[-1].split('.')[0]),'w') as f:
        for i in range(len(ppc)):
            f.write("{}\n".format(ppc[i]))

    if(group=='PC'):
        f0 = rpc
    elif(group=='PPC'):
        f0 = ppc
    return f0
def get_feature(wave_fn, sylboundaries):
    y, sr = librosa.load(wave_fn, sr=16000)
    S_ir = librosa.stft(y=y, n_fft=512, hop_length=160)
    S_dB = librosa.amplitude_to_db(np.abs(S_ir), ref=np.max)
    spec = S_dB.transpose()[:,1:64]

    fw=81
    half_fw=int((fw-1)/2)
    idx_arr = np.array([[x] for x in range(-half_fw,half_fw+1)])

    while len(spec)<fw:
        spec=np.concatenate((spec,spec[-1:,:]),axis=0)

    syltoks = [syl.split(' ') for syl in sylboundaries.split('\n')[:-1]]
    feas=[]
    #print(syltoks)
    for i in range(len(syltoks)):
        if(syltoks[i][2] == 'ϵ'):
            continue
        else:
            fea = np.zeros((81,65))
            ct = round((float(syltoks[i][0]) + float(syltoks[i][1])) * 50)
            #print(ct)
            lb = ct - half_fw
            rb = ct + half_fw + 1
            sf=int(round(float(syltoks[i][0])*100))
            ef=int(round(float(syltoks[i][1])*100))
            if(ct >= half_fw and len(spec) - ct > half_fw):
                sf = sf - ct + half_fw
                ef = ef - ct + half_fw
            elif(len(spec) - ct <= half_fw):
                sf = sf - (len(spec) - fw)
                ef = ef - (len(spec) - fw)
                rb = len(spec)
                lb = rb - fw
            else:
                lb = 0
                rb = fw
            identify_arr=np.array([[1 if x >= sf and x <= ef else 0] for x in range(fw)])
            slice_spec=spec[lb:rb,:]
            #print(identify_arr.shape,sf,ef)
            fea[:,:1] = idx_arr
            fea[:,1:2] = identify_arr
            fea[:,2:] = slice_spec
            
            feas.append(fea)
    return np.array(feas)

def get_postrior(feas, sylboundaries):
    syltoks = [syl.split(' ') for syl in sylboundaries.split('\n')[:-1]]
    syls=[]
    #print(syltoks)
    for i in range(len(syltoks)):
        if(syltoks[i][2] == 'ϵ'):
            continue
        else:
            syls.append(syltoks[i][2])

    predrest=rec_model.predict(feas,verbose=0)
    #print(predrest)
    str_predrest = ""
    for i in range(len(predrest)):
        #str_predrest_tok = " ".join(map(str, (predrest[i]*100).astype(int)))
        str_predrest_tok = " ".join(["{:.8f}".format(float(x)) for x in predrest[i]])
        str_predrest = str_predrest + syls[i] + ': ' + str_predrest_tok + '\n'
    return str_predrest



recvdir='/home/cewarman/research/my-app/local/recv_data'
wavedir='{}/wavs'.format(recvdir)
textdir='{}/txts'.format(recvdir)
labdir='{}/labs'.format(recvdir)
tmpdir='{}/tmp'.format(recvdir)
pitchdir='{}/pitch'.format(recvdir)
#os.system("mkdir -p {} && mkdir -p {} && mkdir -p {} && mkdir -p {} && mkdir -p {} ".format(wavedir,textdir,labdir,tmpdir,pitchdir))
sessions = {}

async def handler(ws):
    profile=''
    group=''
    wavid=''
    repeat_time=0
    context=''
    async for message in ws:
        # 注意：message 可能是 str（文字）或 bytes（二進位）
        if isinstance(message, bytes):
            fn=wavid+"_"+group+"_"+repeat_time
            wave_fn="{}/{}.wav".format(sessions[profile]['wavedir'],fn)
            with open(wave_fn, "wb") as f:
                f.write(message)
            with open("{}/{}.txt".format(sessions[profile]['textdir'],fn), "w") as f:
                f.write(context)
            print(group,wavid,context)
            wavlstpath='{}/{}_wavlst.txt'.format(sessions[profile]['tmpdir'],wavid)
            txtlstpath='{}/{}_txtlst.txt'.format(sessions[profile]['tmpdir'],wavid)
            os.system('echo {}/{}.wav > {} '.format(sessions[profile]['wavedir'],fn,wavlstpath))
            os.system('echo {}/{}.txt > {} '.format(sessions[profile]['textdir'],fn,txtlstpath))
            os.system('cd /home/cewarman/research/kaldi/CEmix/TAAN_pitch && python3 gmm_syl_realign.py {} {} {}'.format(txtlstpath,wavlstpath,sessions[profile]['labdir']))
            #clear_output()
            lab_fn="{}/{}.lab".format(sessions[profile]['labdir'],fn)
            if Path(lab_fn).is_file():
                sylboundaries=syllable_boundary_normalize(lab_fn)
                feas = get_feature(wave_fn, sylboundaries)
                await ws.send("syllable_boundary:"+sylboundaries)
                f0 = get_pitch(group, wave_fn, feas, sylboundaries, sessions[profile]['pitchdir'])
                
                str_f0 = " ".join(map(str, f0))
                await ws.send("pitch:"+str(str_f0))
                #print(f0)

                postrior = get_postrior(feas, sylboundaries)
                print(postrior)
                await ws.send("postrior:"+str(postrior))
        else:
            print(message)
            if(message[:5]=='group'):
                group=message[5:]
                sessions[profile]['group']=group
            elif(message[:7]=='profile'):
                profile=message[7:]
                if profile in sessions:
                    # 找到之前的狀態，繼續處理
                    user_state = sessions[profile]
                    print(f"歡迎回來 {profile}")
                else:
                    sessions[profile] = {}
                    sessions[profile]['wavedir']='{}/{}/wavs'.format(recvdir, profile)
                    sessions[profile]['textdir']='{}/{}/txts'.format(recvdir, profile)
                    sessions[profile]['labdir']='{}/{}/labs'.format(recvdir, profile)
                    sessions[profile]['tmpdir']='{}/{}/tmp'.format(recvdir, profile)
                    sessions[profile]['pitchdir']='{}/{}/pitch'.format(recvdir, profile)
                    os.system("mkdir -p {} && mkdir -p {} && mkdir -p {} && mkdir -p {} && mkdir -p {} ".format(sessions[profile]['wavedir'],sessions[profile]['textdir'],sessions[profile]['labdir'],sessions[profile]['tmpdir'],sessions[profile]['pitchdir']))
                    sessions[profile]['group']=profile.split('_')[-1]
                    print("新連線")
            elif(message[:5]=='wavid'):
                wavid=message[5:].split()[0]
                repeat_time=message[5:].split()[1]
            elif(message[:7]=='context'):
                context=message[7:]
            elif(message[:6]=='bdinfo'):
                sylboundaries=message.split("|")[3]
                readtimes=message.split("|")[2]
                expfile=message.split("|")[1]
                wave_fn=sessions[profile]['wavedir']+"/"+profile[:profile.rfind('_')+1]+expfile+"_"+sessions[profile]['group']+"_"+readtimes+".wav"
                lab_fn=sessions[profile]['labdir']+"/"+profile[:profile.rfind('_')+1]+expfile+"_"+sessions[profile]['group']+"_"+readtimes+'_revised'+".lab"
                print(sylboundaries)
                print(wave_fn)
                with open(lab_fn,'w') as f:
                    f.write(sylboundaries)
                feas = get_feature(wave_fn, sylboundaries)
                f0 = get_pitch(sessions[profile]['group'], wave_fn, feas, sylboundaries, sessions[profile]['pitchdir'])
                str_f0 = " ".join(map(str, f0))
                await ws.send("pitch:"+str(str_f0))
                postrior = get_postrior(feas, sylboundaries)
                await ws.send("postrior:"+str(postrior))
            elif(message[:15]=='self_evaluation'):
                #print(message)
                with open(recvdir+'/'+profile+'/self_evaluation.txt', 'a') as f:
                    f.write('{}\n'.format(message[16:]))


async def main():
    async with websockets.serve(handler, "127.0.0.1", 9999):
        print("Listening on ws://127.0.0.1:9999")
        await asyncio.Future()



try:
    asyncio.run(main())
except RuntimeError:
    # 如果已經有 event loop（像在 Jupyter）
    import nest_asyncio
    nest_asyncio.apply()
    asyncio.get_event_loop().run_until_complete(main())