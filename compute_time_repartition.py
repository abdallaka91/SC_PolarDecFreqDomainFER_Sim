#!/usr/bin/env python3
"""
run_sim.py — Lance Sim2 pour différentes valeurs de K et stocke les résultats.

Usage :
    python run_sim.py --decoder scl_zc_f32_t --list 4 --kvalues 16 24 32 48

Arguments :
    --decoder   Valeur de ?DECODEUR?  (ex: scl_zc_f32_t)
    --list      Valeur entière de ?LIST?
    --kvalues   Liste des valeurs de K (séparées par des espaces)
    --output    Fichier de sortie (défaut: results.txt)
    --sim       Chemin vers le simulateur (défaut: ./build/Sim2)
"""

import argparse
import re
import subprocess
import sys
from datetime import datetime
from pathlib import Path


# ---------------------------------------------------------------------------
# Configuration de la commande
# ---------------------------------------------------------------------------

FIXED_ARGS = [
    "-target", "-6.00",
    "-snr-min", "-4.00",
    "-snr-max", "-4.00",
    "-snr-step", "0.25",
    "-q", "64",
    "-N", "64",
    "-cores", "1",
    "-time-limit", "10",
]


def build_command(sim: str, decoder: str, list_val: int, k: int) -> list[str]:
    dec_arg = f"{decoder}{{{list_val}-{list_val}-8}}"
    return [sim, *FIXED_ARGS, "-K", str(k), "-dec", dec_arg]


# ---------------------------------------------------------------------------
# Parsing de la sortie de Sim2
# ---------------------------------------------------------------------------

PATTERN = {
    "sum":  re.compile(r"sum_exec_time\s*=\s+[\d.]+\s+([\d.]+)"),
    "leaf": re.compile(r"t_exec_leaf\s*=\s+[\d.]+\s+([\d.]+)"),
    "sort": re.compile(r"t_exec_sort\s*=\s+[\d.]+\s+([\d.]+)"),
    "copy": re.compile(r"t_exec_copy\s*=\s+[\d.]+\s+([\d.]+)"),
}


def parse_output(raw: str) -> dict | None:
    values = {}
    for key, pattern in PATTERN.items():
        m = pattern.search(raw)
        values[key] = float(m.group(1)) if m else None
    return None if any(v is None for v in values.values()) else values


# ---------------------------------------------------------------------------
# Formatage
# ---------------------------------------------------------------------------

def format_row(k: int, v: dict) -> str:
    s, l, so, c = v["sum"], v["leaf"], v["sort"], v["copy"]
    t_graph = round(s - l, 3)
    t_leaf  = round(l - so - c, 3)
    return (
        f"{k:<6}  {s:<8.3f}  {t_graph:<10.3f}  {t_leaf:<10.3f}  {so:<8.3f}  {c:<8.3f}"
    )


HEADER = (
    f"{'K':<6}  {'Sum':<8}  {'tGraph':<10}  {'tLeaf':<10}  {'tSort':<8}  {'tCopy':<8}\n"
    + "-" * 60
)


# ---------------------------------------------------------------------------
# Point d'entrée
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Lance Sim2 pour plusieurs valeurs de K.")
    parser.add_argument("--decoder", required=True, help="Nom du décodeur (ex: scl_zc_f32_t)")
    parser.add_argument("--list",    required=True, type=int, help="Valeur de LIST (entier)")
    parser.add_argument("--kvalues", required=True, type=int, nargs="+", help="Valeurs de K")
    parser.add_argument("--output",  default="results.txt", help="Fichier de sortie")
    parser.add_argument("--sim",     default="./build/Sim2", help="Chemin du simulateur")
    parser.add_argument("--dry-run", action="store_true",   help="Affiche les commandes sans les exécuter")
    args = parser.parse_args()

    output_path = Path(args.output)

    print(f"Décodeur : {args.decoder}  |  LIST : {args.list}")
    print(f"Valeurs de K : {args.kvalues}")
    if args.dry_run:
        print("\n[DRY RUN] Commandes qui seraient lancées :\n")
        for k in args.kvalues:
            cmd = build_command(args.sim, args.decoder, args.list, k)
            print(" ".join(cmd))
        return

    # Toujours repartir d'un fichier vierge
    with output_path.open("w", encoding="utf-8") as f:
        f.write(f"# Sim2 — {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}"
                f"  |  Décodeur : {args.decoder}  |  LIST : {args.list}\n\n")
        f.write(HEADER + "\n")

    print(f"Sortie : {output_path}\n")
    print(HEADER)

    for k in args.kvalues:
        cmd = build_command(args.sim, args.decoder, args.list, k)
        print(f"  Lancement K={k}...", end=" ", flush=True)

        try:
            proc = subprocess.run(cmd, capture_output=True, text=True, timeout=240)
            raw = proc.stdout + proc.stderr
            values = parse_output(raw)

            if values is None:
                msg = f"{'K='+str(k):<6}  [ERREUR: parsing incomplet]"
                print(msg)
            else:
                msg = format_row(k, values)
                print(msg)

            with output_path.open("a", encoding="utf-8") as f:
                f.write(msg + "\n")

        except subprocess.TimeoutExpired:
            msg = f"{'K='+str(k):<6}  [ERREUR: timeout]"
            print(msg)
            with output_path.open("a", encoding="utf-8") as f:
                f.write(msg + "\n")

        except FileNotFoundError:
            print(f"\n[FATAL] Simulateur introuvable : {args.sim}", file=sys.stderr)
            sys.exit(1)

    print(f"\nRésultats écrits dans : {output_path}")


if __name__ == "__main__":
    main()
