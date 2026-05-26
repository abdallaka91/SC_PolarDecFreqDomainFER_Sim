#!/usr/bin/env python3
"""
Script de lancement des simulations Sim2.
Itère sur les valeurs de -q et -N, avec K = round(R * N).
Extrait N, K, GF, T.avg depuis la sortie et les enregistre dans un fichier texte.
"""

import subprocess
import argparse
import sys
import re
from itertools import product


# ---------------------------------------------------------------------------
# Règles de validation
# ---------------------------------------------------------------------------

RULES = """
╔══════════════════════════════════════════════════════════════════════╗
║              RÈGLES D'UTILISATION — run_sim2.py                     ║
╠══════════════════════════════════════════════════════════════════════╣
║  Paramètres OBLIGATOIRES (aucune valeur par défaut) :               ║
║                                                                      ║
║  --target    FLOAT   Seuil cible (ex: -8.00)                        ║
║  --snr-min   FLOAT   SNR minimum  (ex: -6.00)                       ║
║  --snr-max   FLOAT   SNR maximum  (ex: -6.00)                       ║
║  --snr-step  FLOAT   Pas du SNR   (ex:  0.25)                       ║
║  --errors    INT     Nombre d'erreurs cible (ex: 1000000)           ║
║  --decoder   STR     Nom du décodeur (ex: scl_zc_f32_t{4-4-8-0})   ║
║                                                                      ║
║  Plages pour -q (entre 8 et 1024) — au moins UNE option :          ║
║    --q-values  INT [INT ...]  Liste explicite  (ex: 8 32 64)        ║
║    --q-min / --q-max [--q-step]  Plage (défaut : puissances de 2)  ║
║                                                                      ║
║  Plages pour -N (entre 64 et 1024) — au moins UNE option :         ║
║    --N-values  INT [INT ...]  Liste explicite  (ex: 64 128 512)     ║
║    --N-min / --N-max [--N-step]  Plage (défaut : puissances de 2)  ║
║                                                                      ║
║  K est calculé automatiquement : K = round(R * N)                   ║
║                                                                      ║
║  Paramètres optionnels :                                             ║
║  --rate        FLOAT Rendement R ∈ ]0,1], K=round(R*N) (défaut:0.5)║
║  --cores       INT  Nombre de cœurs           (défaut : 1)          ║
║  --time-limit  INT  Limite de temps en sec.   (défaut : 60)         ║
║  --output      STR  Fichier de résultats      (défaut : results.txt)║
║  --executable  STR  Chemin vers Sim2          (défaut: ./build/Sim2)║
║  --dry-run         Affiche les commandes sans les exécuter          ║
║  --stop-on-error   Arrête au premier échec                          ║
║                                                                      ║
║  Exemple complet :                                                   ║
║    python run_sim2.py                                                ║
║      --target -8.00 --snr-min -6.00 --snr-max -6.00 --snr-step 0.25║
║      --errors 1000000 --decoder "scl_zc_f32_t{4-4-8-0}"            ║
║      --q-min 8 --q-max 1024                                         ║
║      --N-min 64 --N-max 1024                                        ║
╚══════════════════════════════════════════════════════════════════════╝
"""

# Sentinel pour détecter les paramètres non fournis
_MISSING = object()


# ---------------------------------------------------------------------------
# Parsing des arguments
# ---------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(
        description="Lance des simulations Sim2 en faisant varier -q et -N.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=RULES,
        add_help=True,
    )

    # ── Paramètres OBLIGATOIRES (pas de default) ──────────────────────────
    parser.add_argument("--target",   type=float, default=None, metavar="FLOAT",
                        help="[OBLIGATOIRE] Seuil cible (ex: -8.00)")
    parser.add_argument("--snr-min",  type=float, default=None, metavar="FLOAT",
                        help="[OBLIGATOIRE] SNR minimum (ex: -6.00)")
    parser.add_argument("--snr-max",  type=float, default=None, metavar="FLOAT",
                        help="[OBLIGATOIRE] SNR maximum (ex: -6.00)")
    parser.add_argument("--snr-step", type=float, default=None, metavar="FLOAT",
                        help="[OBLIGATOIRE] Pas du SNR (ex: 0.25)")
    parser.add_argument("--errors",   type=int,   default=None, metavar="INT",
                        help="[OBLIGATOIRE] Nombre d'erreurs cible (ex: 1000000)")
    parser.add_argument("--decoder",  type=str,   default=None, metavar="STR",
                        help="[OBLIGATOIRE] Nom du décodeur (ex: scl_zc_f32_t{4-4-8-0})")

    # ── Plages -q ──────────────────────────────────────────────────────────
    parser.add_argument("--q-min",    type=int, default=None, metavar="INT",
                        help="Valeur minimale de -q (entre 8 et 1024)")
    parser.add_argument("--q-max",    type=int, default=None, metavar="INT",
                        help="Valeur maximale de -q (entre 8 et 1024)")
    parser.add_argument("--q-step",   type=int, default=None, metavar="INT",
                        help="Pas entre les valeurs de -q (défaut : puissances de 2)")
    parser.add_argument("--q-values", type=int, nargs="+",    metavar="INT",
                        help="Liste explicite de valeurs de -q (ex: 8 32 64)")

    # ── Plages -N ──────────────────────────────────────────────────────────
    parser.add_argument("--N-min",    type=int, default=None, metavar="INT",
                        help="Valeur minimale de -N (entre 64 et 1024)")
    parser.add_argument("--N-max",    type=int, default=None, metavar="INT",
                        help="Valeur maximale de -N (entre 64 et 1024)")
    parser.add_argument("--N-step",   type=int, default=None, metavar="INT",
                        help="Pas entre les valeurs de -N (défaut : puissances de 2)")
    parser.add_argument("--N-values", type=int, nargs="+",    metavar="INT",
                        help="Liste explicite de valeurs de -N (ex: 64 128 512)")

    # ── Paramètres optionnels ──────────────────────────────────────────────
    parser.add_argument("--rate",       type=float, default=0.5,
                        help="Rendement R ∈ ]0,1] : K = round(R * N) (défaut : 0.5)")
    parser.add_argument("--cores",      type=int, default=1,
                        help="Nombre de cœurs (défaut : 1)")
    parser.add_argument("--time-limit", type=int, default=60,
                        help="Limite de temps par simulation en secondes (défaut : 60)")
    parser.add_argument("--output",     type=str, default="results.txt",
                        help="Fichier de résultats (défaut : results.txt)")
    parser.add_argument("--executable", type=str, default="./build/Sim2",
                        help="Chemin vers l'exécutable Sim2 (défaut : ./build/Sim2)")
    parser.add_argument("--dry-run",    action="store_true",
                        help="Affiche les commandes sans les exécuter")
    parser.add_argument("--stop-on-error", action="store_true",
                        help="Arrête le script si une commande échoue")

    return parser.parse_args()


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

def validate(args):
    errors = []

    # Paramètres obligatoires simples
    mandatory = [
        ("--target",   args.target,   "Seuil cible,  ex: --target -8.00"),
        ("--snr-min",  args.snr_min,  "SNR minimum,  ex: --snr-min -6.00"),
        ("--snr-max",  args.snr_max,  "SNR maximum,  ex: --snr-max -6.00"),
        ("--snr-step", args.snr_step, "Pas du SNR,   ex: --snr-step 0.25"),
        ("--errors",   args.errors,   "Nb d'erreurs, ex: --errors 1000000"),
        ("--decoder",  args.decoder,  'Décodeur,     ex: --decoder "scl_zc_f32_t{4-4-8-0}"'),
    ]
    for flag, val, hint in mandatory:
        if val is None:
            errors.append(f"  {flag:12s}  manquant  →  {hint}")

    if not (0 < args.rate <= 1):
        errors.append(
            f"  --rate={args.rate}  invalide  →  R doit être dans ]0, 1]  (ex: --rate 0.5)"
        )

    # Plage -q : il faut --q-values OU (--q-min ET --q-max)
    has_q = bool(args.q_values)
    has_q_range = (args.q_min is not None) or (args.q_max is not None)

    if not has_q and not has_q_range:
        errors.append(
            "  --q-values ou (--q-min + --q-max)  manquants\n"
            "             ex: --q-min 8 --q-max 1024\n"
            "             ex: --q-values 8 32 64 256"
        )
    elif has_q_range and not has_q:
        if args.q_min is None:
            errors.append("  --q-min  manquant  (fournir avec --q-max,  ex: --q-min 8)")
        if args.q_max is None:
            errors.append("  --q-max  manquant  (fournir avec --q-min,  ex: --q-max 1024)")
        if args.q_min is not None and args.q_min < 8:
            errors.append(f"  --q-min={args.q_min}  invalide  →  valeur minimale autorisée : 8")
        if args.q_max is not None and args.q_max > 1024:
            errors.append(f"  --q-max={args.q_max}  invalide  →  valeur maximale autorisée : 1024")

    if has_q:
        bad = [v for v in args.q_values if not (8 <= v <= 1024)]
        if bad:
            errors.append(f"  --q-values : valeurs hors plage [8, 1024] : {bad}")

    # Plage -N : il faut --N-values OU (--N-min ET --N-max)
    has_N = bool(args.N_values)
    has_N_range = (args.N_min is not None) or (args.N_max is not None)

    if not has_N and not has_N_range:
        errors.append(
            "  --N-values ou (--N-min + --N-max)  manquants\n"
            "             ex: --N-min 64 --N-max 1024\n"
            "             ex: --N-values 64 128 512"
        )
    elif has_N_range and not has_N:
        if args.N_min is None:
            errors.append("  --N-min  manquant  (fournir avec --N-max,  ex: --N-min 64)")
        if args.N_max is None:
            errors.append("  --N-max  manquant  (fournir avec --N-min,  ex: --N-max 1024)")
        if args.N_min is not None and args.N_min < 64:
            errors.append(f"  --N-min={args.N_min}  invalide  →  valeur minimale autorisée : 64")
        if args.N_max is not None and args.N_max > 1024:
            errors.append(f"  --N-max={args.N_max}  invalide  →  valeur maximale autorisée : 1024")

    if has_N:
        bad = [v for v in args.N_values if not (64 <= v <= 1024)]
        if bad:
            errors.append(f"  --N-values : valeurs hors plage [64, 1024] : {bad}")

    if errors:
        print("\n[ERREUR] Paramètre(s) manquant(s) ou invalide(s) :\n")
        for e in errors:
            print(e)
        print(RULES)
        sys.exit(1)


# ---------------------------------------------------------------------------
# Utilitaires
# ---------------------------------------------------------------------------

def powers_of_two_range(lo: int, hi: int):
    values, v = [], 1
    while v <= hi:
        if v >= lo:
            values.append(v)
        v *= 2
    return values


def build_range(explicit, lo, hi, step):
    if explicit:
        return sorted(explicit)
    if step is not None:
        return list(range(lo, hi + 1, step))
    return powers_of_two_range(lo, hi)


def build_command(args, q: int, N: int) -> list:
    K = max(1, round(args.rate * N))
    return [
        args.executable,
        "-target",   str(args.target),
        "-snr-min",  str(args.snr_min),
        "-snr-max",  str(args.snr_max),
        "-snr-step", str(args.snr_step),
        "-q",        str(q),
        "-N",        str(N),
        "-K",        str(K),
        "-dec",         args.decoder,
        "-cores",       str(args.cores),
        "-errors",      str(args.errors),
        "-time-limit",  str(args.time_limit),
    ]


# ---------------------------------------------------------------------------
# Extraction des données depuis la sortie du programme
# ---------------------------------------------------------------------------

_RE_N    = re.compile(r'#\(DD\) N\s+:\s+(\d+)')
_RE_K    = re.compile(r'#\(DD\) K\s+:\s+(\d+)')
_RE_GF   = re.compile(r'#\(DD\) GF\(q\)\s+:\s+(\d+)')
_RE_DATA = re.compile(
    r'^\s*(-?\d+\.\d+)\s*\|'   # SNR
    r'\s*\d+\s*\|'             # F.Errs
    r'\s*\d+\s*\|'             # frames
    r'\s*[\d.e+\-]+\s*\|'      # FER
    r'\s*\d+\s*\|'             # E.Time
    r'\s*[\d\-]+\s*\|'         # R.Time
    r'\s*([\d.]+)',             # T.avg
    re.MULTILINE,
)


def extract_results(output: str):
    m_N  = _RE_N.search(output)
    m_K  = _RE_K.search(output)
    m_GF = _RE_GF.search(output)
    if not (m_N and m_K and m_GF):
        return []
    N, K, GF = m_N.group(1), m_K.group(1), m_GF.group(1)
    matches = list(_RE_DATA.finditer(output))
    if not matches:
        return []
    # On ne conserve que la dernière ligne de résultat
    m = matches[-1]
    return [dict(N=N, K=K, GF=GF, SNR=m.group(1), T_avg=m.group(2))]


def _write_results(path: str, rows: list):
    with open(path, "w") as f:
        f.write(f"{'N':>6}  {'K':>6}  {'GF':>6}  {'T.avg':>8}\n")
        f.write("#" + "-" * 33 + "\n")
        for r in rows:
            f.write(f"{r['N']:>6}  {r['K']:>6}  {r['GF']:>6}  {r['T_avg']:>8}\n")


# ---------------------------------------------------------------------------
# Programme principal
# ---------------------------------------------------------------------------

def main():
    args = parse_args()
    validate(args)

    q_lo = args.q_min if args.q_min is not None else 8
    q_hi = args.q_max if args.q_max is not None else 1024
    N_lo = args.N_min if args.N_min is not None else 64
    N_hi = args.N_max if args.N_max is not None else 1024

    q_values = build_range(args.q_values, q_lo, q_hi, args.q_step)
    N_values = build_range(args.N_values, N_lo, N_hi, args.N_step)

    if not q_values:
        print("[ERREUR] Aucune valeur de -q dans la plage spécifiée.", file=sys.stderr)
        sys.exit(1)
    if not N_values:
        print("[ERREUR] Aucune valeur de -N dans la plage spécifiée.", file=sys.stderr)
        sys.exit(1)

    total = len(q_values) * len(N_values)
    print(f"=== Sim2 batch launcher ===")
    print(f"Décodeur    : {args.decoder}")
    print(f"Rendement R : {args.rate}  (K = round(R × N))")
    print(f"Valeurs q   : {q_values}")
    print(f"Valeurs N   : {N_values}")
    print(f"Simulations : {total}")
    print(f"Fichier out : {args.output}")
    print(f"Mode        : {'DRY-RUN' if args.dry_run else 'EXECUTION'}")
    print()

    failures = 0
    all_rows = []

    def print_progress(idx, total, q, N, status=""):
        pct    = idx / total * 100
        filled = int(pct / 2)
        bar    = "█" * filled + "░" * (50 - filled)
        suffix = f"  {status}" if status else ""
        print(f"\r[{bar}] {pct:5.1f}%  ({idx}/{total})  q={q} N={N}{suffix}",
              end="", flush=True)

    for idx, (q, N) in enumerate(product(q_values, N_values), start=1):
        K   = N // 2
        cmd = build_command(args, q, N)

        if args.dry_run:
            print_progress(idx, total, q, N, "DRY-RUN")
            print(f"\n  $ {' '.join(cmd)}")
            continue

        print_progress(idx, total, q, N, "en cours…")

        lines = []
        proc  = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in proc.stdout:
            lines.append(line)
        proc.wait()
        output = "".join(lines)

        if proc.returncode != 0:
            failures += 1
            print_progress(idx, total, q, N, f"ÉCHEC (code {proc.returncode})")
            print(f"\n  Commande : {' '.join(cmd)}", file=sys.stderr)
            if args.stop_on_error:
                print("  Arrêt demandé (--stop-on-error).", file=sys.stderr)
                _write_results(args.output, all_rows)
                sys.exit(proc.returncode)
        else:
            rows = extract_results(output)
            if rows:
                all_rows.extend(rows)
                print_progress(idx, total, q, N, "OK")
            else:
                print_progress(idx, total, q, N, "OK (aucune donnée extraite)")

    print()

    _write_results(args.output, all_rows)

    print(f"Terminé : {total - failures}/{total} succès — "
          f"{len(all_rows)} ligne(s) enregistrée(s) dans '{args.output}'")
    if failures:
        sys.exit(1)


if __name__ == "__main__":
    main()
