#!/usr/bin/env bash
# Usage : ./scripts/projet.sh [compile|upload|monitor|test|diagnostic|diagnostic-sd|memoire] [release|debug]
set -euo pipefail
cd "$(dirname "$0")/.."
action=${1:-compile}
mode=${2:-release}
if command -v pio >/dev/null 2>&1; then pio_cmd=pio
elif [[ -x "$HOME/.platformio/penv/bin/pio" ]]; then pio_cmd="$HOME/.platformio/penv/bin/pio"
else printf 'Installer PlatformIO Core avant de continuer.\n' >&2; exit 1; fi
case "$mode" in release|debug) ;; *) printf 'Mode attendu : release ou debug\n' >&2; exit 1;; esac
case "$action" in
 compile) "$pio_cmd" run -e "$mode" ;;
 upload) "$pio_cmd" run -e "$mode" -t upload ;;
 monitor) "$pio_cmd" device monitor -b 115200 ;;
 diagnostic) "$pio_cmd" run -e diagnostic -t upload ;;
 diagnostic-sd) "$pio_cmd" run -e diagnostic_sd_grove -t upload ;;
 test) ./scripts/test.sh ;;
 memoire) python3 scripts/memoire.py "$mode" ;;
 *) printf 'Action inconnue : %s\n' "$action" >&2; exit 1 ;;
esac
