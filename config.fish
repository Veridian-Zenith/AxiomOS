# =========================================================
# Base
# =========================================================
source /usr/share/cachyos-fish-config/cachyos-config.fish
fish_add_path $HOME/.spicetify

set -gx CLICOLOR 1
set -gx COLORTERM truecolor
set -gx MANPAGER "less -R"

# =========================================================
# Fish Syntax Colors (neutral but modern)
# =========================================================
set -U fish_color_command brmagenta
set -U fish_color_keyword brpurple
set -U fish_color_param yellow
set -U fish_color_error red
set -U fish_color_cwd bryellow
set -U fish_color_autosuggestion 555
set -U fish_color_comment brblack
set -U fish_color_operator brcyan
set -U fish_color_escape brblue
set -U fish_color_quote brgreen

# =========================================================
# LLVM / Clang Toolchain (Pure LLD)
# =========================================================
set -Ux CC clang
set -Ux CXX clang++
set -Ux LD ld.lld
set -Ux AR llvm-ar
set -Ux NM llvm-nm
set -Ux STRIP llvm-strip
set -Ux OBJCOPY llvm-objcopy
set -Ux OBJDUMP llvm-objdump
set -Ux READELF llvm-readelf
set -Ux AS clang
set -Ux HOSTCC clang
set -Ux HOSTCXX clang++

# =========================================================
# Compiler & Debug Flags
# =========================================================
set -Ux COMMON_FLAGS "-march=native -O3 -pipe -fno-plt \
-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function \
-Wno-unused-but-set-variable -Wno-missing-field-initializers \
-Wno-sign-compare -Wno-unused-result -fexperimental-new-pass-manager"

# Debug / Sanitizers
set -Ux DEBUG_FLAGS "-g -fstandalone-debug -fsanitize=undefined,address,leak \
-fsanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls"

set -Ux CFLAGS "$COMMON_FLAGS $DEBUG_FLAGS"
set -Ux CXXFLAGS "$COMMON_FLAGS $DEBUG_FLAGS"
set -Ux CPPFLAGS "-D_FORTIFY_SOURCE=3"
set -Ux LTOFLAGS "-flto=thin"

# =========================================================
# Linker Flags (Pure LLD)
# =========================================================
set -Ux LDFLAGS "-fuse-ld=lld -Wl,-O1,--as-needed,-z,relro,-z,now"
set -Ux LLVM_PARALLEL_LINK_JOBS (nproc)
set -Ux LLVM_ENABLE_LLD 1
set -Ux LLVM_ENABLE_LTO thin

# =========================================================
# Rust Toolchain (LLVM-only)
# =========================================================
set -Ux RUSTFLAGS "-C linker=clang -C link-arg=-fuse-ld=lld -C lto=thin \
-A dead_code -A unused_variables -A unused_imports"

set -Ux CARGO_INCREMENTAL 0
set -Ux CARGO_TARGET_DIR "$HOME/.cargo/build"
set -Ux RUST_BACKTRACE 1

# =========================================================
# Build Systems & Ninja Integration
# =========================================================
set -Ux CMAKE_C_FLAGS $CFLAGS
set -Ux CMAKE_CXX_FLAGS $CXXFLAGS
set -Ux CMAKE_EXE_LINKER_FLAGS $LDFLAGS
set -Ux CMAKE_SHARED_LINKER_FLAGS $LDFLAGS

# Ninja parallelism
set -Ux NINJA_STATUS "[%f/%t] %e "
set -Ux NINJAFLAGS "-j (nproc)"

set -Ux MAKEFLAGS "-s"

# =========================================================
# Defaults (Hyprland)
# =========================================================
set -gx terminal kitty
set -gx fileManager nemo
set -gx menu "rofi -show drun"
set -gx browser naver-whale-stable
set -gx textEditor mousepad

# =========================================================
# Modern File Listing
# =========================================================
if type -q eza
    alias ls  "eza -lh --icons --group-directories-first"
    alias ll  "eza -lah --icons --group-directories-first --git"
    alias la  "eza -a --icons"
    alias tree "eza --tree --icons"
else
    alias ls  "ls -lh --color=auto"
    alias ll  "ls -lah --color=auto"
    alias la  "ls -a --color=auto"
end

alias grep "grep --color=auto"
alias diff "diff --color=auto"
alias ip "ip -color=auto"

# =========================================================
# Utilities
# =========================================================
function oneapi
    bass source /opt/intel/oneapi/setvars.sh
end

alias vtune-gui 'env ELECTRON_OZONE_PLATFORM_HINT=x11 vtune-gui'
alias trim 'sudo fstrim -av'

# =========================================================
# Memory Maintenance
# =========================================================
function cmem
    echo "Syncing..."
    sudo sync

    echo "Dropping page cache..."
    echo 3 | sudo tee /proc/sys/vm/drop_caches >/dev/null

    if systemctl is-active --quiet systemd-zram-setup@zram0
        echo "Restarting zram..."
        sudo systemctl restart systemd-zram-setup@zram0
    else if systemctl list-units --type=swap | grep -q active
        echo "Restarting swap target..."
        sudo systemctl restart swap.target
    else
        echo "No swap devices active."
    end

    echo "Memory cleanup complete."
end

# =========================================================
# Package Management
# =========================================================
alias upd         'paru -Syu --noconfirm; and flatpak update -y'
alias fwupd       'fwupdmgr refresh; and fwupdmgr upgrade'
alias add         'paru -S'
alias search      'paru -Ss'
alias del         'paru -Rns --noconfirm'
alias purge       'paru -Rns --noconfirm; and paru -Qtdq | paru -Rns -'
alias orphans     'paru -Qtd'
alias list        'paru -Qe'
alias info        'paru -Si'
alias info-local  'paru -Qi'
alias cache-clean 'sudo rm -rf /var/cache/pacman/pkg/download-*; and paru -Sc --noconfirm'

# =========================================================
# GCR SSH Agent
# =========================================================
if not set -q SSH_AUTH_SOCK
    set -xg SSH_AUTH_SOCK /run/user/(id -u)/gcr/ssh
end

while not test -S $SSH_AUTH_SOCK
    sleep 0.1
end

starship init fish | source
