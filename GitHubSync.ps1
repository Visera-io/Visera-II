param (
    [string]$Message
)

if (-not $Message) {
    Write-Host "❌ Please provide a commit message."
    exit 1
}

# Stage all changes
git add -A

# Commit with the provided message
git commit -m "$Message"

# Push to the current branch
git push