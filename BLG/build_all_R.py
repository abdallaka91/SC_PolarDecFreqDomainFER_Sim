import argparse
import os
import subprocess
import sys
#
#
#
#
#
def generate_config_header(N, GF):
    header_content = f"""#ifndef CONFIG_CODE_H
#define CONFIG_CODE_H
#include "codes/N{N}_GF{GF}.hpp"
#endif
"""
    with open("../src/definitions/code.hpp", "w") as f:
        f.write(header_content)
#
#
#
#
#
def compile_project(rate):
        log_file = "/tmp/process.log"
        with open(log_file, "w") as f:
            result = subprocess.run(["make","clean"], stdout=f, stderr=subprocess.STDOUT)
            if result.returncode != 0:
                print(f"❌ Erreur: échec de la compilation (code retour {result.returncode}).")
                sys.exit(1)
            print("🛠️  Compilation [code generator]...")
            result = subprocess.run(["make","code_generator"], stdout=f, stderr=subprocess.STDOUT)
            if result.returncode != 0:
                print(f"❌ Erreur: échec de la compilation (code retour {result.returncode}).")
                sys.exit(1)
            print(f"🚀 Generation du décodeur dédié...")
            result = subprocess.run(["./code_generator", "--code-rate", f"0.{rate}", "--no-verbose"], stdout=f, stderr=subprocess.STDOUT)
            if result.returncode != 0:
                print(f"❌ Erreur: échec de la generation (code retour {result.returncode}).")
                sys.exit(1)
            print("🛠️  Compilation [benchmark]...")
            result = subprocess.run(["make","benchmarking"], stdout=f, stderr=subprocess.STDOUT)
            if result.returncode != 0:
                print(f"❌ Erreur: échec de la compilation (code retour {result.returncode}).")
                sys.exit(1)
#
#
#
#
#
def run_executable(N, GF, R, decoder, platform, cores, time, log_dir):
    executable = "./benchmarking"
    log_file = os.path.join(log_dir, f"{decoder}_N{N}_GF{GF}_R{R}_{platform}.log")
    cmd = [executable, "--decoder", decoder, "--no-color", "--code-rate", f"0.{R}", "--cores" , cores, "--time", time]

    print(f"🚀 Exécution: {cmd} pour N={N} et GF={GF} et R={R}")
    with open(log_file, "w") as f:
        result = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            print(f"❌ Erreur: échec de l'exécution pour N={N} (code retour {result.returncode}).")
            sys.exit(1)
#
#
#
#
#
def generate_report(log_dir, decoder, platform, GF, N, Rs):
    report_file = os.path.join(log_dir, f"R_{decoder}_{platform}.txt")
    with open(report_file, "w") as report:
        header = "   N    K    R   GF   Coded    Info   Lat"
        report.write(header + "\n")

        for R in Rs:
            log_file = os.path.join(log_dir, f"{decoder}_N{N}_GF{GF}_R{R}_{platform}.log")
            if os.path.isfile(log_file):
                with open(log_file, "r") as f:
                    lines = f.readlines()
                    if lines:
                        last_line = lines[-1]#.strip()
                        report.write(f"{last_line}")#\n")
                    else:
                        report.write(f"{N} MISSING_DATA\n")
            else:
                report.write(f"{N} MISSING_LOG\n")

    print(f"📄 Rapport généré: {report_file}")
#
#
#
#
#
def main():
    parser = argparse.ArgumentParser(description="Compile, exécute et génère un rapport de benchmarking.")
    parser.add_argument("--decoder",  required=True, choices=["dec1", "dec2", "dec3", "dec4", "dec5"], help="Nom du décodeur (ex: dec1)")
    parser.add_argument("--platform", required=True, help="Nom de la plateforme pour le nommage du log")
    parser.add_argument("--cores",    required=True, help="Nombre de coeurs actifs")
    parser.add_argument("--time",     required=True, help="Temps de run pour la mesure")
    args = parser.parse_args()

    GFs = [64,  16, 256]
    Ns  = [64, 256,  32]
    Rs =  [20, 25, 30, 35, 40, 50, 60, 66, 70, 75, 82, 90]

    log_dir = "log"
    os.makedirs(log_dir, exist_ok=True)
    for x in range(0, 3):
        N =  Ns [x]
        GF = GFs[x]
        for R in Rs:
            print(f"🔧 Génération config pour N={N}, GF={GF}, R=0.{R}")
            generate_config_header(N, GF)

            compile_project(R)

            run_executable(N, GF, R, args.decoder, args.platform, args.cores, args.time, log_dir)

        generate_report(log_dir, args.decoder, args.platform, GF, N, Rs)

        print(f"✅ Sous-partie n°{x+1} terminée")

    print("✅ Tout est terminé.")
#
#
#
#
#
if __name__ == "__main__":
    main()
#
#
#
#
#
