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
5. � **Deploys directly to GitHub Pages** (without committing to repo)

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
3. **GitHub Pages is automatically updated** with new docs (no commits to repo)

## 📋 Requirements

- Repository must have **"Allow GitHub Actions to create and approve pull requests"** enabled in Settings
- **GitHub Pages must be enabled** in repository Settings → Pages → Source: "GitHub Actions"
- Branch `main` should be set as the default branch
- **No `docs/` folder needed in repository** - documentation is generated and deployed automatically

## ⚙️ Setup Instructions

1. Go to your repository **Settings** → **Pages**
2. Under **Source**, select **"GitHub Actions"**
3. Go to **Settings** → **Actions** → **General**
4. Enable **"Allow GitHub Actions to create and approve pull requests"**
5. Push code changes to trigger the workflows
