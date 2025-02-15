#!/bin/bash

# Prompt for confirmation before resetting the project
read -p "⚠️ This will erase all local changes and sync with the remote repository. Proceed? (y/N) " confirm
if [[ "$confirm" =~ ^[Yy]$ ]]; then
    echo "🔄 Resetting the project to match the remote repository..."

    # Discard all local changes in tracked files
    git reset --hard HEAD

    # Remove all untracked files and directories
    git clean -fd

    # Fetch the latest changes from the remote repository
    git fetch --all

    # Force reset the local branch to match the remote branch
    git reset --hard origin/$(git rev-parse --abbrev-ref HEAD)

    # Pull the latest updates (fast-forward only, no merge conflicts)
    git pull origin $(git rev-parse --abbrev-ref HEAD) --ff-only

    echo "✅ Project has been reset successfully."
else
    echo "❌ Operation canceled."
fi
