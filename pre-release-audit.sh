#!/usr/bin/env bash
# Run cheap release gates before cutting a beta tag.

set -euo pipefail

usage() {
    cat <<'EOF'
usage: ./pre-release-audit.sh <previous-tag-or-ref> [release-tag]

Examples:
  ./pre-release-audit.sh beta-6.1
  ./pre-release-audit.sh beta-7 beta-7.1

The second argument is optional. If it exists, it must point at HEAD.
If it does not exist yet, the script treats HEAD as the release candidate.
EOF
}

if [[ $# -lt 1 || $# -gt 2 ]]; then
    usage >&2
    exit 2
fi

BASE_REF="$1"
RELEASE_REF="${2:-}"

if ! git rev-parse --verify "${BASE_REF}^{commit}" >/dev/null 2>&1; then
    echo "ERROR: base ref does not exist: ${BASE_REF}" >&2
    exit 1
fi

BASE_SHA="$(git rev-parse --short=12 "${BASE_REF}^{commit}")"
HEAD_SHA="$(git rev-parse --short=12 HEAD)"

echo "== Release candidate =="
echo "base: ${BASE_REF} (${BASE_SHA})"
echo "head: HEAD (${HEAD_SHA})"

if [[ -n "${RELEASE_REF}" ]]; then
    if git rev-parse --verify "${RELEASE_REF}^{commit}" >/dev/null 2>&1; then
        RELEASE_SHA="$(git rev-parse --short=12 "${RELEASE_REF}^{commit}")"
        echo "release tag: ${RELEASE_REF} (${RELEASE_SHA})"
        if [[ "${RELEASE_SHA}" != "${HEAD_SHA}" ]]; then
            echo "ERROR: ${RELEASE_REF} does not point at HEAD." >&2
            echo "Cut a new point-release tag or move only an unpublished local tag." >&2
            exit 1
        fi
    else
        echo "release tag: ${RELEASE_REF} (not created yet)"
    fi
fi

echo
echo "== Worktree =="
if [[ -n "$(git status --short)" ]]; then
    git status --short
    echo "ERROR: release audit requires a clean worktree." >&2
    exit 1
fi
echo "clean"

echo
echo "== Diff size =="
git diff --shortstat "${BASE_REF}..HEAD" -- game include platform main.c CMakeLists.txt build.sh build.bat package-linux.sh || true

echo
echo "== Whitespace =="
git diff --check

echo
echo "== Build =="
./build.sh
if [[ -x build/ctr_native ]]; then
    ./build/ctr_native --version
fi

if [[ -f .ctr-progress/ctr_progress.py ]]; then
    echo
    echo "== Progress ledger =="
    python3 .ctr-progress/ctr_progress.py validate
fi

if [[ -f .ctr-progress/tools/data_parity.py && -x build/ctr_native && -f ../CTR-ModSDK/build/ctr-u/SCUS_944.26 ]]; then
    echo
    echo "== Global data parity =="
    python3 .ctr-progress/tools/data_parity.py \
        --native build/ctr_native \
        --retail ../CTR-ModSDK/build/ctr-u/SCUS_944.26 \
        --symbol data \
        --max-runs 40
fi

scan_diff() {
    local title="$1"
    local pattern="$2"
    local tmp

    tmp="$(mktemp)"
    git diff -U0 "${BASE_REF}..HEAD" -- game include platform main.c \
        | rg "${pattern}" >"${tmp}" || true

    local count
    count="$(wc -l <"${tmp}")"

    echo
    echo "== Risk scan: ${title} =="
    echo "matches: ${count}"
    if [[ "${count}" -gt 0 ]]; then
        sed -n '1,120p' "${tmp}"
        if [[ "${count}" -gt 120 ]]; then
            echo "... ${count} total; inspect the full '${title}' pattern in pre-release-audit.sh."
        fi
    fi

    rm -f "${tmp}"
}

if command -v rg >/dev/null 2>&1; then
    scan_diff "packed/matrix copies and narrow stores" \
        '^\+[^+].*(memcpy|memmove|sizeof\([^)]*(MATRIX|matrix|struct)|MatrixRotate|ConvertRotToMatrix|MatrixND|CTR_WriteU(16|32)LE|CTR_ReadU(16|32)LE|= \*|\(MATRIX \*\))'

    scan_diff "native divergence gates" \
        '^\+[^+].*(CTR_NATIVE|#if defined\(CTR_NATIVE\)|#ifdef CTR_NATIVE)'

    scan_diff "retail state width/type edits" \
        '^\+[^+].*(typedef|enum|struct|s8|u8|s16|u16|s32|u32|b32|bool)'
else
    echo
    echo "WARN: rg not found; skipped risky-pattern scans."
fi

echo
echo "Release audit completed. Risk-scan matches are audit prompts, not automatic proof."
