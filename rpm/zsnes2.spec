Name:           zsnes2
Version:        2.3.4
Release:        1%{?dist}
Summary:        ZSNES 2 Super Nintendo emulator

License:        GPL-2.0-only AND LGPL-2.1-or-later AND Zlib
URL:            https://github.com/xyproto/zsnes
Source0:        %{name}-%{version}.tar.gz

# ZSNES 2.3.4 is C11 throughout and supports these Linux targets upstream.
# Keep the Fedora package constrained to architectures with an explicit
# upstream build mapping instead of silently attempting unsupported arches.
ExclusiveArch:  x86_64 i686 aarch64 riscv64

%ifarch x86_64
%global zsnes_bits 64
%global zsnes_cpu x86
%endif
%ifarch i686
%global zsnes_bits 32
%global zsnes_cpu x86
%endif
%ifarch aarch64
%global zsnes_bits 64
%global zsnes_cpu arm64
%endif
%ifarch riscv64
%global zsnes_bits 64
%global zsnes_cpu riscv64
%endif

BuildRequires:  binutils
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  python3
BuildRequires:  pkgconf-pkg-config
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(sdl3)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  desktop-file-utils
# Upstream intentionally exercises the portable x86 test suite as 32-bit
# even when the emulator itself is built as x86_64.
%ifarch x86_64
BuildRequires:  glibc-devel(x86-32)
%endif

%description
ZSNES 2 is a maintained continuation of the classic ZSNES emulator,
updated to build and run on modern Unix-like systems while retaining its
low-overhead implementation and familiar interface.

The current codebase is C11 throughout and supports native 64-bit x86,
AArch64 and RISC-V builds in addition to 32-bit x86. The Fedora package
provides SDL3 software rendering and OpenGL rendering. Game ROM images are
not included.

%prep
%autosetup -n %{name}-%{version}

%build
# Preserve Fedora's compiler/linker hardening where it is compatible with
# upstream's architecture policy. Upstream deliberately builds x86 non-PIE
# with -fno-pic/-no-pie, while arm64 and riscv64 remain position-independent.
%set_build_flags
%make_build \
    ARCH=LINUX \
    BITS=%{zsnes_bits} \
    CPU=%{zsnes_cpu} \
    BINARY=zsnes2 \
    EXTRA_CFLAGS='-DZCONF=\"zsnes2\"' \
    WITH_PIPEWIRE= \
    WITH_AO=

%install
install -Dm0755 zsnes2 %{buildroot}%{_bindir}/zsnes2

for icon_size in 16x16 32x32 48x48 64x64 128x128; do
    install -Dm0644 "img/${icon_size}x32.png" \
        "%{buildroot}%{_datadir}/icons/hicolor/${icon_size}/apps/io.github.xyproto.zsnes2.png"
done

install -d %{buildroot}%{_datadir}/applications
sed \
    -e 's/^Name=ZSNES$/Name=ZSNES 2/' \
    -e 's/^Exec=zsnes /Exec=zsnes2 /' \
    -e 's/^TryExec=zsnes$/TryExec=zsnes2/' \
    -e 's/^Icon=io\.github\.xyproto\.zsnes$/Icon=io.github.xyproto.zsnes2/' \
    -e 's/^StartupWMClass=zsnes$/StartupWMClass=zsnes2/' \
    linux/zsnes.desktop \
    > %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes2.desktop

install -d %{buildroot}%{_datadir}/metainfo
sed \
    -e 's/io\.github\.xyproto\.zsnes/io.github.xyproto.zsnes2/g' \
    -e 's#<name>ZSNES</name>#<name>ZSNES 2</name>#' \
    -e 's#<binary>zsnes</binary>#<binary>zsnes2</binary>#' \
    linux/io.github.xyproto.zsnes.metainfo.xml \
    > %{buildroot}%{_datadir}/metainfo/io.github.xyproto.zsnes2.metainfo.xml

install -d %{buildroot}%{_mandir}/man1
sed \
    -e 's/^ZSNES -/zsnes2 -/' \
    -e 's/^\.B ZSNES$/.B zsnes2/' \
    -e 's/^ZSNES$/zsnes2/' \
    -e 's#~/.zsnes#~/.config/zsnes2#g' \
    man/zsnes.1 \
    > %{buildroot}%{_mandir}/man1/zsnes2.1

%check
%make_build test \
    ARCH=LINUX \
    BITS=%{zsnes_bits} \
    CPU=%{zsnes_cpu} \
    BINARY=zsnes2 \
    EXTRA_CFLAGS='-DZCONF=\"zsnes2\"' \
    WITH_PIPEWIRE= \
    WITH_AO=

# Verify the installed executable's ABI directly from the ELF header.
elf_header="$(readelf -hW %{buildroot}%{_bindir}/zsnes2)"
case "%{_arch}" in
    x86_64)
        printf '%s\n' "$elf_header" | grep -Eq 'Class:[[:space:]]+ELF64'
        printf '%s\n' "$elf_header" | grep -Eq 'Machine:[[:space:]]+Advanced Micro Devices X86-64'
        ;;
    i686)
        printf '%s\n' "$elf_header" | grep -Eq 'Class:[[:space:]]+ELF32'
        printf '%s\n' "$elf_header" | grep -Eq 'Machine:[[:space:]]+Intel 80386'
        ;;
    aarch64)
        printf '%s\n' "$elf_header" | grep -Eq 'Class:[[:space:]]+ELF64'
        printf '%s\n' "$elf_header" | grep -Eq 'Machine:[[:space:]]+AArch64'
        ;;
    riscv64)
        printf '%s\n' "$elf_header" | grep -Eq 'Class:[[:space:]]+ELF64'
        printf '%s\n' "$elf_header" | grep -Eq 'Machine:[[:space:]]+RISC-V'
        ;;
esac

if ldd %{buildroot}%{_bindir}/zsnes2 | grep -q 'not found'; then
    echo 'Unresolved runtime dependency:' >&2
    ldd %{buildroot}%{_bindir}/zsnes2 >&2
    exit 1
fi

desktop-file-validate \
    %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes2.desktop

grep -qx 'Name=ZSNES 2' \
    %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes2.desktop
grep -qx 'Exec=zsnes2 %f' \
    %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes2.desktop
grep -qx 'TryExec=zsnes2' \
    %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes2.desktop
grep -qx 'Icon=io.github.xyproto.zsnes2' \
    %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes2.desktop
grep -q '<id>io.github.xyproto.zsnes2</id>' \
    %{buildroot}%{_datadir}/metainfo/io.github.xyproto.zsnes2.metainfo.xml
grep -q '<binary>zsnes2</binary>' \
    %{buildroot}%{_datadir}/metainfo/io.github.xyproto.zsnes2.metainfo.xml

%files
%license COPYING
%doc README.md
%{_bindir}/zsnes2
%{_datadir}/applications/io.github.xyproto.zsnes2.desktop
%{_datadir}/metainfo/io.github.xyproto.zsnes2.metainfo.xml
%{_datadir}/icons/hicolor/16x16/apps/io.github.xyproto.zsnes2.png
%{_datadir}/icons/hicolor/32x32/apps/io.github.xyproto.zsnes2.png
%{_datadir}/icons/hicolor/48x48/apps/io.github.xyproto.zsnes2.png
%{_datadir}/icons/hicolor/64x64/apps/io.github.xyproto.zsnes2.png
%{_datadir}/icons/hicolor/128x128/apps/io.github.xyproto.zsnes2.png
%{_mandir}/man1/zsnes2.1*

%changelog
* Sun Sep 27 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.3.4-1
- Update to ZSNES 2 2.3.4.
- Adapt the package to upstream's C11 multi-architecture Linux build interface.
- Remove the obsolete NASM and i686-only packaging assumptions.
- Update icon installation for the upstream img/ directory layout.
- Run the upstream test suite, including its 32-bit x86 tests on x86_64.

* Mon Sep 07 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.2.3-1
- Initial Fedora package.
- Package ZSNES 2 separately as zsnes2 so it can coexist with historical zsnes.
- Build the supported 32-bit x86 target with SDL3 and OpenGL support.
- Allow the required non-PIC text relocations on Fedora 44 i686.
