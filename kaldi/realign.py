import os
import sys

if(len(sys.argv) != 4):
	print("Usage: python3 realign.py text_list wave_list output_data_dir")
	exit()

maximum_thread_number = os.cpu_count() -2


bin_realign_prepare="/home/cewarman/research/chores/kaldi_DIY/bin/realign_prepare"
alignment_dir="/home/cewarman/research/kaldi/CEmix/2025/data_alignment"
alignment_data_dir="/home/cewarman/research/kaldi/CEmix/2025/data_alignment/alignment"
ivectors_dir="/home/cewarman/research/kaldi/CEmix/2025/data_alignment/alignment/ivectors"
lexicnop="/home/cewarman/research/kaldi/CEmix/2025/data_alignment/local/dict/lexiconp.txt"
mfcc_dir="/mnt/slave3/kaldi_mfcc/raw"
os.system("rm -rf {} {}".format(mfcc_dir, alignment_data_dir))
os.system("mkdir -p {} && mkdir -p {}".format(alignment_data_dir, sys.argv[3]))
os.system("{} {} {} {} {}".format(bin_realign_prepare, sys.argv[1], sys.argv[2], lexicnop, alignment_data_dir))
os.system("bash ./cmd.sh && . ./path.sh && ./utils/utt2spk_to_spk2utt.pl {}/utt2spk > {}/spk2utt && fix_data_dir.sh {}".format(alignment_data_dir, alignment_data_dir, alignment_data_dir))
with open('{}/spk2utt'.format(alignment_data_dir), 'r', encoding='utf8') as f:
	spknum = len([line.strip() for line in f.readlines()])
thread_num = spknum if spknum < maximum_thread_number else maximum_thread_number
print("spknum = {}".format(spknum))
os.system("steps/make_mfcc.sh --nj {} --mfcc-config conf/mfcc_hires.conf --cmd run.pl {} {}/mfcc_log {}".format(thread_num, alignment_data_dir, alignment_data_dir, mfcc_dir))
os.system("steps/compute_cmvn_stats.sh {} {}/mfcc_log {}".format(alignment_data_dir, alignment_data_dir, mfcc_dir))
os.system("steps/online/nnet2/extract_ivectors_online.sh --cmd run.pl --nj {} {} exp/nnet3/extractor {}".format(thread_num, alignment_data_dir, ivectors_dir))
os.system("steps/nnet3/align.sh --use-gpu false --nj {} --online-ivector-dir {} {} data/lang/ exp/nnet3/tdnn_lstm1c_sp/ {}/alires".format(thread_num, ivectors_dir, alignment_data_dir, alignment_data_dir))
os.system("gunzip -c {}/alires/ali.*.gz > {}/alires/allali.txt".format(alignment_data_dir, alignment_data_dir))
os.system("bash -c '. ./cmd.sh' && . ./path.sh && show-alignments {}/alires/phones.txt {}/alires/final.mdl ark:{}/alires/allali.txt > {}/alires/show_align.txt".format(alignment_data_dir, alignment_data_dir, alignment_data_dir, alignment_data_dir))
with open('{}/text'.format(alignment_data_dir), 'r', encoding='utf8') as f:
	t=[line.strip().split() for line in f.readlines()]
	td={}
	for i in range(len(t)):
		td[t[i][0]]=t[i][1:]
with open('{}/alires/show_align.txt'.format(alignment_data_dir), 'r', encoding='utf8') as f:
	sa=[line.strip() for line in f.readlines()]
	sa_lst=[]
	for i in range(len(sa)):
		if(len(sa[i])==0):
			continue
		sa_lst.append(sa[i])
print('success ali({}/{})'.format(round(len(sa_lst)*0.5),len(td)))
for i in range(round(len(sa_lst)*0.5)):
	st=sa_lst[2*i]
	ps=sa_lst[2*i+1].split()[1:]
	fn=st.split()[0]
	stt=st.split('[')[1:]
	widx=0
	sidx=0
	phone_ranges=[]
	for j in range(len(stt)):
		phlen=len(stt[j][:-2].split())
		eidx=sidx+phlen
		phone_range="{} {} {}".format(round(sidx*0.01,2), round(eidx*0.01,2), ps[j])
		if(ps[j]=='SIL'):
			phone_range=phone_range+" <SILENCE>"
		elif(ps[j][-2:]=='_B' or ps[j][-2:]=='_S'):
			phone_range=phone_range+" "+td[fn][widx]
			widx=widx+1
		sidx=eidx
		phone_ranges.append(phone_range)
	with open('{}/{}.lab'.format(sys.argv[3], fn),'w') as f:
		for j in range(len(phone_ranges)):
			f.write("{}\n".format(phone_ranges[j]))
#with open('{}/alires/ali_phones.txt'.format(alignment_data_dir), 'r', encoding='utf8') as f:
#	aps=[line.strip().split() for line in f.readlines()]
#with open('{}/alires/phones.txt'.format(alignment_data_dir),'r',encoding='utf8') as f:
#	p=[line.strip().split() for line in f.readlines()]
#	pdf2ph={}
#	for i in range(len(p)):
#		pdf2ph[p[i][1]]=p[i][0]
#print("success ali ({}/{})".format(len(aps), len(t)))
#for i in range(len(aps)):
#	fn=aps[i][0]
#	pdfseq=aps[i][1:]
#	widx=0
#	#print(td[fn])
#	uniqpdfseq=[]
#	for j in range(len(pdfseq)-1):
#		if(pdfseq[j]!=pdfseq[j+1]):
#			uniqpdfseq.append([pdfseq[j],j+1])
#	uniqpdfseq.append([pdfseq[-1],len(pdfseq)])
#	#print(uniqpdfseq)
#	phseq=[[pdf2ph[uniqpdfseq[0][0]], uniqpdfseq[0][1]]]
#	for j in range(1, len(uniqpdfseq)):
#		phseq.append([pdf2ph[uniqpdfseq[j][0]], uniqpdfseq[j][1] - uniqpdfseq[j-1][1]])
#	#print(phseq)
#	wordphmlf=[]
#	mlf=[]
#	sidx=0
#	for j in range(len(phseq)):
#		wordphmlf.append([uniqpdfseq[j][1],phseq[j][0]])
#		if(phseq[j][0][-2:]=="_S" or phseq[j][0][-2:]=="_E"):
#			#print(wordphmlf)
#			mlf.append([sidx, wordphmlf[0][0], wordphmlf[0][1], td[fn][widx]])
#			sidx = wordphmlf[0][0]
#			widx = widx+1
#			for k in range(1, len(wordphmlf)):
#				mlf.append([sidx, wordphmlf[k][0], wordphmlf[k][1]])
#				sidx = wordphmlf[k][0]
#			wordphmlf=[]
#		if(phseq[j][0][-2:]!="_S" and phseq[j][0][-2:]!="_B" and phseq[j][0][-2:]!="_I" and phseq[j][0][-2:]!="_E"):
#			#print(wordphmlf)
#			mlf.append([sidx, wordphmlf[0][0], wordphmlf[0][1], '<SILENCE>'])
#			sidx=wordphmlf[0][0]
#			wordphmlf=[]
#	#print(mlf)
#	with open('{}/{}.lab'.format(sys.argv[3], fn),'w') as f:
#		for j in range(len(mlf)):
#			outline="{} {} ".format(round(mlf[j][0]*0.01,3), round(mlf[j][1]*0.01,3))
#			if(mlf[j][2][-2:]=='_S' or mlf[j][2][-2:]=='_B' or mlf[j][2][-2:]=='_I' or mlf[j][2][-2:]=='_E'):
#				outline=outline+mlf[j][2][:-2]
#			for k in range(3, len(mlf[j])):
#				outline=outline+' '+mlf[j][k]
#			#print(outline)
#			#print(mlf[j])
#			f.write("{}\n".format(outline))
#