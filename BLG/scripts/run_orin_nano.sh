# 1 core experiments

python3 ../build_all_N.py  --decoder dec3 --platform orin_nano_1c --cores 1 --time 30
python3 ../build_all_N.py  --decoder dec4 --platform orin_nano_1c --cores 1 --time 30
python3 ../build_all_N.py  --decoder dec5 --platform orin_nano_1c --cores 1 --time 30

python3 ../build_all_GF.py --decoder dec3 --platform orin_nano_1c --cores 1 --time 30
python3 ../build_all_GF.py --decoder dec4 --platform orin_nano_1c --cores 1 --time 30
python3 ../build_all_GF.py --decoder dec5 --platform orin_nano_1c --cores 1 --time 30

python3 ../build_all_R.py  --decoder dec3 --platform orin_nano_1c --cores 1 --time 30
python3 ../build_all_R.py  --decoder dec4 --platform orin_nano_1c --cores 1 --time 30
python3 ../build_all_R.py  --decoder dec5 --platform orin_nano_1c --cores 1 --time 30

# 4 cores experiments

python3 ../build_all_N.py  --decoder dec3 --platform orin_nano_6c --cores 6 --time 30
python3 ../build_all_N.py  --decoder dec4 --platform orin_nano_6c --cores 6 --time 30
python3 ../build_all_N.py  --decoder dec5 --platform orin_nano_6c --cores 6 --time 30

python3 ../build_all_GF.py --decoder dec3 --platform orin_nano_6c --cores 6 --time 30
python3 ../build_all_GF.py --decoder dec4 --platform orin_nano_6c --cores 6 --time 30
python3 ../build_all_GF.py --decoder dec5 --platform orin_nano_6c --cores 6 --time 30

python3 ../build_all_R.py  --decoder dec3 --platform orin_nano_6c --cores 6 --time 30
python3 ../build_all_R.py  --decoder dec4 --platform orin_nano_6c --cores 6 --time 30
python3 ../build_all_R.py  --decoder dec5 --platform orin_nano_6c --cores 6 --time 30
