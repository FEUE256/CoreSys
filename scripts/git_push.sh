#!/bin/bash
set -euo pipefail

# ---------- Progress bar ----------
progress() {
    local msg="$1"
    local delay="${2:-0.3}"
    echo -ne "$msg"
    for i in {1..3}; do
        echo -ne "."
        sleep "$delay"
    done
    echo " done"
}

# Resolve absolute repo root
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Repository root: $REPO_DIR"
cd "$REPO_DIR"

progress "Checking git repository"

git rev-parse --is-inside-work-tree >/dev/null 2>&1 || {
    echo "Error: Not a git repository"
    exit 1
}

BRANCH="$(git symbolic-ref --short HEAD 2>/dev/null || true)"
if [ -z "$BRANCH" ]; then
    echo "Error: Detached HEAD"
    exit 1
fi

progress "Checking remote origin"

git remote get-url origin >/dev/null 2>&1 || {
    echo "Error: Remote 'origin' missing"
    exit 1
}

# Ask commit message
read -r -p "Enter commit message (or press Enter for default): " COMMIT_MSG
COMMIT_MSG="${COMMIT_MSG:-Quick commit at $(date +"%Y-%m-%d %H:%M:%S")}"

progress "Staging files"
git add -A

if git diff --cached --quiet; then
    echo "Nothing to commit."
    exit 0
fi

progress "Creating commit"
git commit -m "$COMMIT_MSG" >/dev/null

progress "Pushing to origin/$BRANCH"
git push origin "$BRANCH" >/dev/null

echo "Git push completed successfully."
