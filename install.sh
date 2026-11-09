#!/usr/bin/env bash
# ==============================================================================
# MatraKi v1.0.0 — AI Smart Bangla Keyboard Installer for Linux
# Developer: Md Sojib Hossain (https://facebook.com/hossain.sojibs)
# Support / Donate: https://www.supportkori.com/sojib
# License: GNU GPL v3.0
# ==============================================================================
set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
USER_DATA_DIR="${HOME}/.local/share/matraki-keyboard"

echo "================================================================"
echo "  MatraKi v1.0.0 — AI Smart Bangla Keyboard Installer"
echo "  Type Smarter. Write Better."
echo "================================================================"

# 1. Install user-level AI dictionary & language model files
echo "[1/4] Installing AI trained dictionaries to ${USER_DATA_DIR}..."
mkdir -p "${USER_DATA_DIR}"
for f in transliteration.json phonetic-candidate-selection.json phonetic-next-word.json snippets.json autocorrect.json; do
    if [ -f "${REPO_DIR}/data/${f}" ] && [ ! -f "${USER_DATA_DIR}/${f}" ]; then
        cp -f "${REPO_DIR}/data/${f}" "${USER_DATA_DIR}/${f}"
    fi
done

# 2. Build Rust core (libriti), Fcitx5 AI Engine (matraki.so), and Qt5 GUI (matraki-gui)
echo "[2/4] Building MatraKi Engine & Modern GUI..."
BUILD_DIR="${REPO_DIR}/build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake "${REPO_DIR}" -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)" matraki-gui

RITI_LIB=$(find "${REPO_DIR}" "${BUILD_DIR}" -name "libriti.a" 2>/dev/null | head -n 1)
if [ -z "${RITI_LIB}" ]; then
    echo "Building libriti..."
    (cd "${REPO_DIR}/src/engine/riti" && cargo build --release)
    RITI_LIB="${REPO_DIR}/src/engine/riti/target/release/libriti.a"
fi

g++ -shared -fPIC -O3 -march=native -std=c++17 \
    -DPROJECT_DATADIR=\"/usr/share/matraki-keyboard\" \
    -DBIN_DIR=\"/usr/share/matraki-keyboard\" \
    -I"${REPO_DIR}/src/engine/fcitx" \
    -I"${REPO_DIR}/src/engine/riti/include" \
    -I/usr/include/Fcitx5/Core \
    -I/usr/include/Fcitx5/Config \
    -I/usr/include/Fcitx5/Utils \
    "${REPO_DIR}/src/engine/fcitx/matraki.cpp" \
    "${RITI_LIB}" \
    -lFcitx5Core -lFcitx5Config -lFcitx5Utils \
    -lpthread -ldl -lm \
    -o "${BUILD_DIR}/matraki.so"

# 3. Install system files (supports standard Linux sudo and Deepin Immutable root)
echo "[3/4] Installing system files and icons..."
INSTALL_CMD="sudo bash -c"
if command -v deepin-immutable-ctl >/dev/null 2>&1; then
    INSTALL_CMD="sudo deepin-immutable-ctl admin exec -- bash -c"
fi

${INSTALL_CMD} "
    set -e
    mkdir -p /usr/share/matraki-keyboard/data /usr/share/matraki-keyboard/layouts /usr/share/matraki-keyboard/icons
    cp -rf '${REPO_DIR}/data/'*.json /usr/share/matraki-keyboard/data/ 2>/dev/null || true
    cp -f '${BUILD_DIR}/src/frontend/matraki-gui' /usr/share/matraki-keyboard/matraki-gui
    chmod +x /usr/share/matraki-keyboard/matraki-gui
    cp -f '${REPO_DIR}/data/128.png' /usr/share/matraki-keyboard/icons/MatraKi.png

    cp -f '${BUILD_DIR}/matraki.so' /usr/lib/x86_64-linux-gnu/fcitx5/matraki.so
    chmod +x /usr/lib/x86_64-linux-gnu/fcitx5/matraki.so
    cp -f '${REPO_DIR}/src/engine/fcitx/matraki.conf' /usr/share/fcitx5/inputmethod/matraki.conf
    cp -f '${REPO_DIR}/src/engine/fcitx/matraki-addon.conf' /usr/share/fcitx5/addon/matraki.conf

    cp -f '${REPO_DIR}/data/matraki.desktop' /usr/share/applications/matraki.desktop
    cp -f '${REPO_DIR}/data/128.png' /usr/share/pixmaps/matraki.png

    for sz in 16 32 48 128 512 1024; do
        if [ -f '${REPO_DIR}/data/'\${sz}'.png' ]; then
            mkdir -p /usr/share/icons/hicolor/\${sz}x\${sz}/apps
            cp -f '${REPO_DIR}/data/'\${sz}'.png' /usr/share/icons/hicolor/\${sz}x\${sz}/apps/matraki.png
        fi
    done
    if [ -d /usr/share/dsg/icons ] && [ -f '${REPO_DIR}/data/matraki.dci' ]; then
        cp -f '${REPO_DIR}/data/matraki.dci' /usr/share/dsg/icons/matraki.dci
    fi
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor 2>/dev/null || true
    update-desktop-database /usr/share/applications 2>/dev/null || true
"

echo "[4/4] Restarting Fcitx5 & Launching MatraKi TopBar..."
killall -9 matraki-gui fcitx5 2>/dev/null || true
fcitx5 -d 2>/dev/null || true
nohup /usr/share/matraki-keyboard/matraki-gui >/dev/null 2>&1 &

echo "================================================================"
echo "  ✓ MatraKi v1.0.0 installed successfully!"
echo "  Press Ctrl+Space to switch between English and Bangla."
echo "================================================================"
