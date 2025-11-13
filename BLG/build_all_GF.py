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
def compile_project():
    log_file = "/tmp/process.log"
    with open(log_file, "w") as f:
        print("🛠️  Compilation initiale (1/2)...")
        result = subprocess.run(["make","clean"], stdout=f, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            print(f"❌ Erreur: échec de la compilation (code retour {result.returncode}).")
            sys.exit(1)
        result = subprocess.run(["make","code_generator"], stdout=f, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            print(f"❌ Erreur: échec de la compilation (code retour {result.returncode}).")
            sys.exit(1)
        print(f"🚀 Generation du décodeur dédié...")
        result = subprocess.run(["./code_generator", "--code-rate", "0.50", "--no-verbose"], stdout=f, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            print(f"❌ Erreur: échec de la generation (code retour {result.returncode}).")
            sys.exit(1)
        print("🛠️  Compilation finale (2/2)...")
        result = subprocess.run(["make","benchmarking"], stdout=f, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            print(f"❌ Erreur: échec de la compilation (code retour {result.returncode}).")
            sys.exit(1)
#
#
#
#
#
def run_executable(N, GF, decoder, platform, cores, time, log_dir):
    executable = "./benchmarking"
    log_file = os.path.join(log_dir, f"{decoder}_N{N}_GF{GF}_{platform}.log")
    cmd = [executable, "--decoder", decoder, "--no-color", "--cores" , cores, "--time", time, "--code-rate", "0.50"]

    print(f"🚀 Exécution: {cmd} pour N={N} et GF={GF}")

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
def generate_report(log_dir, decoder, platform, GF, N):
    report_file = os.path.join(log_dir, f"GF_{decoder}_{platform}.txt")
    with open(report_file, "w") as report:
        header = "   N    K    R   GF   Coded    Info   Lat"
        report.write(header + "\n")

        for gf in GF:
            log_file = os.path.join(log_dir, f"{decoder}_N{N}_GF{gf}_{platform}.log")
            if os.path.isfile(log_file):
                with open(log_file, "r") as f:
                    lines = f.readlines()
                    if lines:
                        last_line = lines[-1]#.strip()
                        report.write(f"{last_line}")#\n")
                    else:
                        report.write(f"{gf} MISSING_DATA\n")
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

    GF = [8, 16, 32, 64, 128, 256, 512, 1024]  # fixe ou tu peux le rendre paramétrable
    N  = 64

    log_dir = "log"
    os.makedirs(log_dir, exist_ok=True)

    for gf in GF:
        print(f"🔧 Génération config pour N={N}, GF={gf}")
        generate_config_header(N, gf)

        compile_project()

        run_executable(N, gf, args.decoder, args.platform, args.cores, args.time, log_dir)

    generate_report(log_dir, args.decoder, args.platform, GF, N)
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