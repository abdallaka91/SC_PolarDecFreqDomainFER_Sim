rm -rf ./log

# Amélioration de notre algo de décodage

#python3 ../build_all_N.py  --decoder dec0 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_N.py  --decoder dec1 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_N.py  --decoder dec3 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_N.py  --decoder dec4 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_N.py  --decoder dec5 --platform macOs_1c --cores 1  --time 30

#python3 ../build_all_GF.py --decoder dec0 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_GF.py --decoder dec1 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_GF.py --decoder dec3 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_GF.py --decoder dec4 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_GF.py --decoder dec5 --platform macOs_1c --cores 1  --time 30

#python3 ../build_all_R.py  --decoder dec0 --platform macOs_1c --cores 1  --time 30
#python3 ../build_all_R.py  --decoder dec1 --platform macOs_1c --cores 1  --time 30
python3 ../build_all_R.py  --decoder dec3 --platform macOs_1c --cores 1  --time 30
python3 ../build_all_R.py  --decoder dec4 --platform macOs_1c --cores 1  --time 30
python3 ../build_all_R.py  --decoder dec5 --platform macOs_1c --cores 1  --time 30

# ABC

#python3 ../build_all_N.py  --decoder dec3 --platform macOs_8c --cores 8  --time 60
#python3 ../build_all_N.py  --decoder dec4 --platform macOs_8c --cores 8  --time 60
#python3 ../build_all_N.py  --decoder dec5 --platform macOs_8c --cores 8  --time 60

#python3 ../build_all_GF.py --decoder dec3 --platform macOs_8c --cores 8  --time 60
#python3 ../build_all_GF.py --decoder dec4 --platform macOs_8c --cores 8  --time 60
#python3 ../build_all_GF.py --decoder dec5 --platform macOs_8c --cores 8  --time 60

python3 ../build_all_R.py  --decoder dec3 --platform macOs_8c --cores 8  --time 60
python3 ../build_all_R.py  --decoder dec4 --platform macOs_8c --cores 8  --time 60
python3 ../build_all_R.py  --decoder dec5 --platform macOs_8c --cores 8  --time 60
