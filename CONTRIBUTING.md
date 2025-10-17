# Contributing to fastcond

## Code Style

This project uses `clang-format` for consistent code formatting. All code must pass format checks before merging.

### Before Committing

**Always run the formatter before committing:**

```bash
./scripts/format.sh
```

This ensures your code will pass CI format checks.

### Manual Formatting

Format all files:
```bash
find fastcond test -name "*.c" -o -name "*.h" | xargs clang-format -i
```

Check formatting without modifying (same as CI):
```bash
find fastcond test -name "*.c" -o -name "*.h" | xargs clang-format -n -Werror
```

### Format Configuration

The project uses `.clang-format` in the root directory with these key settings:
- Based on LLVM style
- 4-space indentation
- 100 character line limit
- Linux-style braces

## Testing

Run the test suite before submitting:

```bash
# Quick smoke tests
./scripts/run_tests.sh quick

# Full test suite
./scripts/run_tests.sh

# With CMake/CTest
ctest --test-dir build --output-on-failure
```

## Continuous Integration

GitHub Actions runs these checks on every push/PR:
- **Build**: gcc and clang on Ubuntu and macOS
- **Tests**: All test variants (pthread, strong, weak)
- **Sanitizers**: AddressSanitizer, ThreadSanitizer, UndefinedBehavior
- **Format**: clang-format verification
- **Static Analysis**: clang-tidy

All checks must pass before merging.

## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. **Run `./scripts/format.sh`** ← Important!
5. Run tests: `./scripts/run_tests.sh`
6. Commit with clear message
7. Push and create pull request
8. Ensure CI passes

## Code Review

- Keep changes focused and atomic
- Include tests for new functionality
- Update documentation as needed
- Follow existing code patterns
- Maintain backward compatibility

## Questions?

Open an issue for discussion before starting major changes.
