. ./cmd.sh
. ./path.sh

stage=0
train=true
decode=true
mfccdir='/mnt/slave3/features/kaldi'

#spk2utt  text  utt2spk  wav.scp
if [ $stage -le 0 ]; then

for x in data/train data/test; do
    utils/fix_data_dir.sh $x || exit 1;
    steps/make_mfcc_pitch.sh --nj 30 --cmd "$train_cmd" $x exp/make_mfcc/${x##*/} $mfccdir || exit 1;
    steps/compute_cmvn_stats.sh $x exp/make_mfcc/${x##*/} $mfccdir || exit 1;
    utils/validate_data_dir.sh $x || exit 1;
done
utils/prepare_lang.sh data/local/dict "<UNK>" data/local/lang data/lang || exit 1;
arpa2fst --disambig-symbol=#0 --read-symbol-table=data/lang/words.txt data/tmpdir/bible.arpa data/lang/G.fst || exit 1;
fi

if [ $stage -le 1 ]; then
    if $train; then
        steps/train_mono.sh --boost-silence 1.25 --nj 30 --cmd "$train_cmd" data/train data/lang exp/mono || exit 1;
    fi
    if $decode; then
        utils/mkgraph.sh data/lang exp/mono exp/mono/graph && \
        nspk=$(wc -l <data/test/spk2utt)
        steps/decode.sh --nj ${nspk} --cmd "$decode_cmd" exp/mono/graph data/test exp/mono/decode || exit 1;
        #steps/decode.sh --nj 20 --cmd "$decode_cmd" exp/mono/graph data/test exp/mono/decode || exit 1;
    fi
fi
if [ $stage -le 2 ]; then
    if $train; then
        steps/align_si.sh --boost-silence 1.25 --nj 30 --cmd "$train_cmd" data/train data/lang exp/mono exp/mono_ali || exit 1;
        steps/train_deltas.sh --boost-silence 1.25 --cmd "$train_cmd" 2000 10000 data/train data/lang exp/mono_ali exp/tri1 || exit 1;
    fi
    if $decode; then
        utils/mkgraph.sh data/lang exp/tri1 exp/tri1/graph || exit 1;
        nspk=$(wc -l <data/test/spk2utt)
        steps/decode.sh --nj ${nspk} --cmd "$decode_cmd" exp/tri1/graph data/test exp/tri1/decode || exit 1;
        #steps/decode.sh --nj 10 --cmd "$decode_cmd" exp/tri1/graph data/test exp/tri1/decode || exit 1;
    fi
fi
if [ $stage -le 3 ]; then
    if $train; then
        steps/align_si.sh --nj 30 --cmd "$train_cmd" data/train data/lang exp/tri1 exp/tri1_ali || exit 1;
        steps/train_lda_mllt.sh --cmd "$train_cmd" --splice-opts "--left-context=3 --right-context=3" 2500 15000 data/train data/lang exp/tri1_ali exp/tri2b || exit 1;
    fi
    if $decode; then
        utils/mkgraph.sh data/lang exp/tri2b exp/tri2b/graph || exit 1;
        nspk=$(wc -l <data/test/spk2utt)
        steps/decode.sh --nj ${nspk} --cmd "$decode_cmd" exp/tri2b/graph data/test exp/tri2b/decode || exit 1;
        #steps/decode.sh --nj 10 --cmd "$decode_cmd" exp/tri2b/graph data/test exp/tri2b/decode || exit 1;
    fi
fi
if [ $stage -le 4 ]; then
  # From 2b system, train 3b which is LDA + MLLT + SAT.
    if $train; then
        steps/align_si.sh  --nj 30 --cmd "$train_cmd" data/train data/lang exp/tri2b exp/tri2b_ali  || exit 1;
        steps/train_sat.sh --cmd "$train_cmd" 4200 40000 data/train data/lang exp/tri2b_ali exp/tri3b || exit 1;
    fi
    if $decode; then
        utils/mkgraph.sh data/lang exp/tri3b exp/tri3b/graph || exit 1;
        nspk=$(wc -l <data/test/spk2utt)
        steps/decode_fmllr.sh --nj 10 --cmd "$decode_cmd" exp/tri3b/graph data/test exp/tri3b/decode || exit 1;
    fi
fi
if [ $stage -le 5 ]; then
    # Estimate pronunciation and silence probabilities.
    # Silprob for normal lexicon.
    steps/get_prons.sh --cmd "$train_cmd" data/train data/lang exp/tri3b || exit 1;
    utils/dict_dir_add_pronprobs.sh --max-normalize true data/local/dict exp/tri3b/pron_counts_nowb.txt exp/tri3b/sil_counts_nowb.txt exp/tri3b/pron_bigram_counts_nowb.txt data/local/dict_sp || exit 1
    utils/prepare_lang.sh data/local/dict_sp "<UNK>" data/local/lang_tmp data/lang || exit 1;
fi
if [ $stage -le 6 ]; then
    # From 3b system, now using data/lang as the lang directory (we have now added
    # pronunciation and silence probabilities), train another SAT system (tri4b).
    if $train; then
        steps/train_sat.sh  --cmd "$train_cmd" 4200 40000 data/train data/lang exp/tri3b exp/tri4b || exit 1;
    fi

    if $decode; then
        utils/mkgraph.sh data/lang exp/tri4b exp/tri4b/graph || exit 1;
        nspk=$(wc -l <data/test/spk2utt)
        steps/decode_fmllr.sh --nj ${nspk} --cmd "$decode_cmd" exp/tri4b/graph data/test exp/tri4b/decode || exit 1;
    fi
fi