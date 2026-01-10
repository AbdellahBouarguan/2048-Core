# Contributing to 2048-Core

We welcome contributions! Please follow these engineering standards to maintain project integrity.

## 🛠 Engineering Standards

### 1. Strict ANSI C (C89)
* All code must compile with `-std=c89 -pedantic`.
* **Variable Declarations**: Must be at the top of the scope (before any logic).
* **Comments**: Use `/* ... */` only. No `//`.
* **No Global State**: Unless absolutely necessary (e.g., simplistic singletons).

### 2. Code Style
* **Indentation**: 4 spaces (no tabs).
* **Braces**: Linux style (Egyptian brackets for functions, but K&R for control structures).
* **Naming**: `snake_case` for variables/functions, `PascalCase` for structs/enums, `UPPER_CASE` for macros.

## 🚀 Workflow

1.  **Fork & Branch**: Create a branch using the convention `type/scope-description`.
    * Example: `feat/input-system`, `fix/memory-leak`.
2.  **Commit Messages**: We follow [Conventional Commits](https://www.conventionalcommits.org/).
    * `feat:`: New feature.
    * `fix:`: Bug fix.
    * `docs:`: Documentation only changes.
    * `chore:`: Build scripts, repo maintenance.
    * *Example*: `feat: implement atomic save game storage`
3.  **Pull Request**: Open a PR to `main`. Ensure CI passes before requesting review.

## 🧪 Testing
Run the local test suite before pushing:
```bash
./scripts/build_local.sh debug
./build/run_tests
```
