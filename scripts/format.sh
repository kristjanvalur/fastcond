#!/bin/bash
# Format all C source files according to .clang-format
# Run this before committing to ensure CI format checks pass

set -e

# Colors
BLUE='\033[0;34m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${BLUE}Formatting C source files...${NC}"

# Find and format all .c and .h files
find fastcond test -name "*.c" -o -name "*.h" | xargs clang-format -i

echo -e "${GREEN}✓ Formatting complete!${NC}"
echo ""
echo "Modified files:"
git status --short | grep -E "^\s*M.*\.(c|h)$" || echo "  (no files changed)"
