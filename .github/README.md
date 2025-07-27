# GitHub Actions Workflows

## 📚 Documentation Workflow (`docs.yml`)

Automatically generates Doxygen documentation whenever:
- Code is pushed to the `main` branch
- Files in `server/workspace/src/` or `server/workspace/include/` are modified
- The `Doxyfile` configuration is updated

### What it does:
1. 🗑️ **Clears the `docs/` folder** - removes all old documentation files
2. 📚 **Installs Doxygen** and GraphViz on Ubuntu 
3. 🔨 **Generates fresh documentation** from current source code
4. ✅ **Verifies** that documentation was generated successfully
5. 📝 **Commits and pushes** changes back to the repository

### Triggers:
- `push` to `main` with changes in source code
- `pull_request` to `main` with changes in source code

---

## 🔨 Build Workflow (`build.yml`)

Tests whether the project compiles on every push and pull request.

### What it does:
1. 📦 **Installs dependencies** - cmake, build-essential, nlohmann-json3-dev
2. 🔨 **Compiles the server** using CMake

### Triggers:
- Every `push` to `main`
- Every `pull_request` to `main`

---

## 🚀 Usage

After pushing code to GitHub:
1. Build workflow checks if code compiles successfully
2. Documentation workflow generates fresh documentation
3. Documentation is automatically committed and pushed back

## 📋 Requirements

- Repository must have **"Allow GitHub Actions to create and approve pull requests"** enabled in Settings
- Branch `main` should be set as the default branch
