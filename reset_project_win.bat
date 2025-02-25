@echo off
title Git Project Reset
echo ⚠️ This will erase all local changes and sync with the remote repository.
echo.
set /p confirm=Proceed? (y/N): 

if /I "%confirm%"=="Y" (
    echo.
    echo 🔄 Resetting the project to match the remote repository...
    
    REM Ensure the script is running inside a Git repository
    git rev-parse --is-inside-work-tree >nul 2>&1
    if %errorlevel% neq 0 (
        echo ❌ Error: This is not a Git repository. Exiting...
        pause
        exit /b
    )

    REM Discard all local changes
    git reset --hard HEAD

    REM Remove all untracked files and directories
    git clean -fd

    REM Fetch the latest changes from the remote repository
    git fetch --all

    REM Get the current branch name
    for /f %%i in ('git rev-parse --abbrev-ref HEAD') do set BRANCH=%%i

    REM Force reset local branch to match the remote branch
    git reset --hard origin/%BRANCH%

    REM Pull the latest updates (fast-forward only)
    git pull origin %BRANCH% --ff-only

    echo.
    echo ✅ Project has been reset successfully.
) else (
    echo ❌ Operation canceled.
)

pause
exit
