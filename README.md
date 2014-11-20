# OpenFst tools

fst_extract_subnet
* Extracts sub-FST from FST given by start and end state

fst_traverser
* General FST traverser


### Query-by-Example Spoken Term Detection:

Assuming we have HTK phone lattices for both queries and utterances, we have to follow these steps:

1. Convert lattice to confusion network (sausage)
```
    scripts/lattice2sausage.sh
```
2. Convert sausages to FSTs
```
    scripts/sausage2fst_*.sh
```
3. Compose query and utterance FSTs
```
    scripts/compose_fst_cn.sh
```
4. Convert composition features to recs
```
    scripts/compose_fst_cn_features_to_recs.sh
```
5. Merge recs to a single MLF file
```
    scripts/compose_fst_rec2mlf.sh
```
> Additional scripts are in the ./scripts/ subdir. Some of them is more convenient to copy to your $PATH dir (e.q. $HOME/bin)
