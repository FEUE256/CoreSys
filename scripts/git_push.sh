#!/bin/bash
set -e

# Resolve absolute repo root (robust in WSL, symlinks, cron, etc.)
REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "Repository root: $REPO_DIR"
cd "$REPO_DIR"

# Sanity check: ensure this is a git repo
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Error: Not a git repository: $REPO_DIR"
    exit 1
fi

# Ask once (no infinite loop)
read -p "Enter commit message (or press Enter for default): " COMMIT_MSG
if [ -z "$COMMIT_MSG" ]; then
    COMMIT_MSG="Quick commit at $(date +"%Y-%m-%d %H:%M:%S")"
fi

# Stage changes
git add .

# Do not fail if there is nothing to commit
if git diff --cached --quiet; then
    echo "Nothing to commit."
    exit 0
fi

# Commit and push
git commit -m "$COMMIT_MSG"
git push origin main

echo "Git push completed with commit: $COMMIT_MSG"
