#!/bin/bash
# Format all source files according to project standards
# Run this before committing to ensure CI format checks pass

set -e

# Colors
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

echo -e "${BLUE}Formatting C source files...${NC}"

# Find and format all .c and .h files
find fastcond test -name "*.c" -o -name "*.h" | xargs clang-format -i

echo -e "${GREEN}✓ C formatting complete!${NC}"

echo -e "${BLUE}Formatting Python source files...${NC}"

# Check if we're in scripts directory or need to change to it
if [ -d "scripts" ]; then
    cd scripts
elif [ ! -f "pyproject.toml" ]; then
    echo -e "${YELLOW}Warning: No scripts directory or pyproject.toml found, skipping Python formatting${NC}"
else
    # We're already in scripts directory
    :
fi

# Format Python files if pyproject.toml exists
if [ -f "pyproject.toml" ]; then
    # Use uvx to run ruff (no virtual environment needed)
    if command -v uvx >/dev/null 2>&1; then
        echo "  - Checking Python code quality..."
        uvx ruff check . --fix || true  # Continue even if there are unfixable issues
        echo "  - Formatting Python files..."
        uvx ruff format .
        echo -e "${GREEN}✓ Python formatting complete!${NC}"
    else
        echo -e "${YELLOW}uvx not found, skipping Python formatting${NC}"
    fi
    cd - > /dev/null  # Return to original directory silently
else
    echo -e "${YELLOW}No Python configuration found, skipping Python formatting${NC}"
fi

echo ""
echo "Modified files:"
git status --short | grep -E "^\s*M.*\.(c|h|py)$" || echo "  (no files changed)"
