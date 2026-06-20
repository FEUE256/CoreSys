#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CONFIG_PATH="$ROOT/.config/cfg.yaml"
OUTPUT_CFG="$ROOT/.config/config.cfg"

echo "ROOT=$ROOT"
echo "CONFIG_PATH=$CONFIG_PATH"
echo "OUTPUT_CFG=$OUTPUT_CFG"

touch "$OUTPUT_CFG"

# ----------------------------
# helpers
# ----------------------------

is_list() {
    # enkel heuristik: YAML list börjar med "-"
    [[ "$(grep -E '^[[:space:]]*-' "$1" || true)" != "" ]]
}

join_flags() {
    # tar YAML-list och gör "a b c"
    awk '
    /^[[:space:]]*-/ {
        gsub(/^[[:space:]]*-[[:space:]]*/, "")
        printf "%s ", $0
    }
    END { print "" }
    ' "$1" | xargs
}

to_make_value() {
    local key="$1"
    local val="$2"

    if [[ "$val" == *"|"* ]]; then
        echo "$val"
    else
        echo "$val"
    fi
}

# ----------------------------
# yaml parser (simple, flat)
# ----------------------------

generate_make() {
    local file="$1"

    while IFS= read -r line; do

        # skip comments / empty
        [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]] && continue

        # key: value
        if [[ "$line" =~ ^[[:space:]]*([^:]+):[[:space:]]*(.*)$ ]]; then
            key="${BASH_REMATCH[1]}"
            val="${BASH_REMATCH[2]}"

            # strip quotes
            val="${val%\"}"
            val="${val#\"}"

            echo "$key := $val"
        fi

    done < "$file"
}

# ----------------------------
# main
# ----------------------------

touch $OUTPUT_CFG

generate_make "$CONFIG_PATH" > "$OUTPUT_CFG"

echo "Wrote: $OUTPUT_CFG"
