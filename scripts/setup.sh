#!/usr/bin/env bash
set -euo pipefail

# ── Constants ─────────────────────────────────────────────────────────────────
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly BUILD_DEBUG_DIR="build/debug"
readonly BUILD_RELEASE_DIR="build/release"
readonly CLANG_FORMAT_FILE="${SCRIPT_DIR}/.clang-format"
readonly CLANG_TIDY_FILE="${SCRIPT_DIR}/.clang-tidy"
readonly GITIGNORE_FILE="${SCRIPT_DIR}/.gitignore"
readonly CLANG_FORMAT_MIN_VERSION=12

# ── Colours ───────────────────────────────────────────────────────────────────
readonly COLOR_GREEN="\033[0;32m"
readonly COLOR_YELLOW="\033[0;33m"
readonly COLOR_RED="\033[0;31m"
readonly COLOR_RESET="\033[0m"

log_info()  { echo -e "${COLOR_GREEN}[INFO]${COLOR_RESET}  $*"; }
log_warn()  { echo -e "${COLOR_YELLOW}[WARN]${COLOR_RESET}  $*"; }
log_error() { echo -e "${COLOR_RED}[ERROR]${COLOR_RESET} $*" >&2; }

# ── Dependency checks ─────────────────────────────────────────────────────────
check_dependency() {
    local cmd="$1"
    local install_hint="$2"

    if ! command -v "$cmd" &>/dev/null; then
        log_error "'$cmd' is not installed. $install_hint"
        exit 1
    fi
}

check_cmake_version() {
    local required_major=3
    local required_minor=16
    local version_string
    version_string=$(cmake --version | head -n1 | grep -oP '\d+\.\d+')
    local major minor
    major=$(echo "$version_string" | cut -d. -f1)
    minor=$(echo "$version_string" | cut -d. -f2)

    if [[ "$major" -lt "$required_major" ]] || \
       [[ "$major" -eq "$required_major" && "$minor" -lt "$required_minor" ]]; then
        log_error "CMake >= ${required_major}.${required_minor} required (found ${version_string})."
        log_error "Install: sudo apt install cmake  OR  pip install cmake"
        exit 1
    fi

    log_info "CMake ${version_string} detected — OK"
}

check_clang_format_version() {
    local version
    version=$(clang-format --version | grep -oP '\d+' | head -n1)

    if [[ "$version" -lt "$CLANG_FORMAT_MIN_VERSION" ]]; then
        log_warn "clang-format >= ${CLANG_FORMAT_MIN_VERSION} recommended (found ${version})."
        log_warn "Some .clang-format options may be ignored."
    else
        log_info "clang-format ${version} detected — OK"
    fi
}

check_clang_tidy_version() {
    local version
    version=$(clang-tidy --version | grep -oP '\d+' | head -n1)

    if [[ "$version" -lt 12 ]]; then
        log_warn "clang-tidy >= 12 recommended (found ${version})."
    else
        log_info "clang-tidy ${version} detected — OK"
    fi
}

# ── .clang-format creation ────────────────────────────────────────────────────
create_clang_format() {
    if [[ -f "$CLANG_FORMAT_FILE" ]]; then
        log_info ".clang-format already exists — skipping creation"
        return
    fi

    log_info "Creating .clang-format..."
    cat > "$CLANG_FORMAT_FILE" << 'EOF'
---
Language: Cpp
BasedOnStyle: LLVM

# Indentation
IndentWidth: 4
TabWidth: 4
UseTab: Never
IndentCaseLabels: true
IndentPPDirectives: BeforeHash

# Line length
ColumnLimit: 100

# Braces
BreakBeforeBraces: Attach

# Spaces
SpaceBeforeParens: ControlStatements
SpaceAfterCStyleCast: false
SpaceBeforeRangeBasedForLoopColon: true
SpacesInAngles: Never
SpacesInContainerLiterals: false

# Alignment
AlignConsecutiveAssignments: Consecutive
AlignConsecutiveDeclarations: Consecutive
AlignTrailingComments:
  Kind: Always
  OverEmptyLines: 1

# Includes
SortIncludes: CaseSensitive
IncludeBlocks: Regroup

# Modern C++
Standard: c++20
Cpp11BracedListStyle: true
SpaceBeforeCpp11BracedList: false

# Penalties (controls line-break decisions)
PenaltyBreakBeforeFirstCallParameter: 100
PenaltyReturnTypeOnItsOwnLine: 200
EOF

    log_info ".clang-format created"
}

create_clang_tidy() {
    if [[ -f "$CLANG_TIDY_FILE" ]]; then
        log_info ".clang-tidy already exists — skipping creation"
        return
    fi

    log_info "Creating .clang-tidy..."
    cat > "$GITIGNORE_FILE" << 'EOF'
---
Checks: >
  clang-diagnostic-*,
  clang-analyzer-*,
  cppcoreguidelines-*,
  modernize-*,
  performance-*,
  readability-*,
  bugprone-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers

WarningsAsErrors: "*"
HeaderFilterRegex: ".*"
FormatStyle: file
EOF

    log_info ".clang-tidy created"
}

# ── .gitignore creation ───────────────────────────────────────────────────────
create_gitignore() {
    if [[ -f "$GITIGNORE_FILE" ]]; then
        log_info ".gitignore already exists — skipping creation"
        return
    fi

    log_info "Creating .gitignore..."
    cat > "$GITIGNORE_FILE" << 'EOF'
# Build directories
build/
out/
cmake-build-*/

# CMake generated files
CMakeCache.txt
CMakeFiles/
CMakeScripts/
CTestTestfile.cmake
cmake_install.cmake
compile_commands.json
install_manifest.txt
_deps/

# Compiled binaries & objects
*.o
*.a
*.so
*.so.*
*.dylib
*.exe
*.out
*.app

# Clang tooling
.cache/
.clangd/
*.plist

# Sanitizer / profiling output
*.profraw
*.profdata
default.profraw

# Editor & IDE
.vscode/
.idea/
*.swp
*.swo
*~
.DS_Store
Thumbs.db

# Logs & temp
*.log
*.tmp
EOF

    log_info ".gitignore created"
}

# ── Git init ──────────────────────────────────────────────────────────────────
init_git_repo() {
    if [[ -d "${SCRIPT_DIR}/.git" ]]; then
        log_info "Git repository already exists — skipping init"
        return
    fi

    check_dependency "git" "Install: sudo apt install git"

    log_info "Initialising git repository..."
    git -C "$SCRIPT_DIR" init

    # Rename default branch to main when git >= 2.28 supports --initial-branch
    local git_version major minor
    git_version=$(git --version | grep -oP '\d+\.\d+' | head -n1)
    major=$(echo "$git_version" | cut -d. -f1)
    minor=$(echo "$git_version" | cut -d. -f2)

    if [[ "$major" -gt 2 ]] || [[ "$major" -eq 2 && "$minor" -ge 28 ]]; then
        git -C "$SCRIPT_DIR" checkout -b main 2>/dev/null || true
    fi

    local branch
    branch=$(git -C "$SCRIPT_DIR" branch --show-current)
    log_info "Git repository initialised on branch '${branch}'"

    git -C "$SCRIPT_DIR" add .
    log_info "Files staged — run 'git commit -m \"Initial commit\"' when ready"
}

# ── CMake configure ───────────────────────────────────────────────────────────
configure_build() {
    local build_dir="$1"
    local build_type="$2"

    log_info "Configuring ${build_type} build -> ${build_dir}/"
    cmake -B "${SCRIPT_DIR}/${build_dir}" \
          -DCMAKE_BUILD_TYPE="$build_type" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          "$SCRIPT_DIR"
}

# ── CMake build ───────────────────────────────────────────────────────────────
build_target() {
    local build_dir="$1"
    local build_type="$2"
    local num_cores
    num_cores=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)

    log_info "Building ${build_type} with ${num_cores} parallel jobs..."
    cmake --build "${SCRIPT_DIR}/${build_dir}" --parallel "$num_cores"
}

# ── Format targets ────────────────────────────────────────────────────────────
build_format_targets() {
    local build_dir="$1"

    log_info "Running clang-format on all sources..."
    cmake --build "${SCRIPT_DIR}/${build_dir}" --target format
}

build_tidy_targets() {
    local build_dir="$1"
    log_info "Running clang-tidy on all sources..."
    cmake --build "${SCRIPT_DIR}/${build_dir}" --target tidy
}

# ── Main ──────────────────────────────────────────────────────────────────────
main() {
    log_info "=== Project setup starting ==="

    # Move to project root so all relative paths resolve correctly
    cd "$SCRIPT_DIR"

    check_dependency "cmake"        "Install: sudo apt install cmake"
    check_dependency "c++"          "Install: sudo apt install g++"
    check_dependency "clang-format" "Install: sudo apt install clang-format"
    check_dependency "clang-tidy"   "Install: sudo apt install clang-tidy"

    check_cmake_version
    check_clang_format_version
    check_clang_tidy_version

    create_clang_format
    create_clang_tidy
    create_gitignore
    init_git_repo

    configure_build "$BUILD_DEBUG_DIR"   "Debug"
    configure_build "$BUILD_RELEASE_DIR" "Release"

    build_target "$BUILD_DEBUG_DIR"   "Debug"
    build_target "$BUILD_RELEASE_DIR" "Release"

    build_format_targets "$BUILD_DEBUG_DIR"
    build_tidy_targets "$BUILD_DEBUG_DIR"

    echo ""
    log_info "=== Setup complete ==="
    log_info "  Debug   executables -> ${BUILD_DEBUG_DIR}/"
    log_info "  Release executables -> ${BUILD_RELEASE_DIR}/"
    echo ""
    log_info "Useful commands:"
    echo "  cmake --build ${BUILD_DEBUG_DIR} --target format        # format in-place"
    echo "  cmake --build ${BUILD_DEBUG_DIR} --target format-check  # CI dry-run check"
    echo "  cmake --build ${BUILD_RELEASE_DIR} -j\$(nproc)           # rebuild release"
    echo "  git commit -m \"Initial commit\"                          # first commit"
}

main "$@"