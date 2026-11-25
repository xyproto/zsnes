Name:           zsnes
Version:        2.3.9
Release:        2%{?dist}
Summary:        ZSNES Super Nintendo emulator

License:        GPL-2.0-only AND LGPL-2.1-or-later AND Zlib AND MIT AND Unlicense AND CC0-1.0
URL:            https://github.com/xyproto/zsnes
Source0:        %{name}-%{version}.tar.gz
Source1:        io.github.xyproto.zsnes.pipewire.desktop
Source2:        io.github.xyproto.zsnes.sdl3.desktop
Source3:        io.github.xyproto.zsnes.libao.desktop

# Upstream supports these Linux targets.
# Keep the Fedora package constrained to architectures with an explicit
# upstream build mapping instead of silently attempting unsupported arches.
ExclusiveArch:  x86_64 i686 aarch64 riscv64

# Upstream builds 32-bit x86 without PIE. The 64-bit targets retain Fedora's
# normal hardening, including PIE.
%ifarch i686
%undefine _hardened_build
%endif

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
BuildRequires:  pkgconfig(ao)
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(libpipewire-0.3)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(sdl3)
BuildRequires:  desktop-file-utils
# Upstream intentionally exercises the portable x86 test suite as 32-bit
# even when the emulator itself is built as x86_64. Fedora's 32-bit GCC
# support also needs the matching libatomic runtime available to the linker.
%ifarch x86_64
BuildRequires:  glibc-devel(x86-32)
BuildRequires:  libatomic(x86-64)
BuildRequires:  libatomic(x86-32)
%endif

Provides:       bundled(miniz) = 3.0.0

%description
ZSNES is a maintained continuation of the classic ZSNES emulator,
updated to build and run on modern Unix-like systems while retaining its
low-overhead implementation and familiar interface.

The current codebase is C11 throughout and supports native 64-bit x86,
AArch64 and RISC-V builds in addition to 32-bit x86. The Fedora package
provides SDL3 and OpenGL rendering, plus SDL3, PipeWire and libao audio.
Game ROM images are not included.

%prep
%autosetup -n %{name}-%{version}
# Include the embedded miniz notices in the binary package.
sed -n '/Copyright 2013-2014/,/THE SOFTWARE\./p; /THE SOFTWARE\./q' miniz.c > miniz-LICENSE
sed -n '/This is free and unencumbered software/,/For more information/p' miniz.c >> miniz-LICENSE
grep -q 'Permission is hereby granted' miniz-LICENSE
grep -q 'unlicense.org' miniz-LICENSE

%build
%set_build_flags
%ifarch i686
# Restore immediate binding when the Fedora PIE specs are disabled.
export LDFLAGS="$LDFLAGS -Wl,-z,now"
%endif
%make_build \
    ARCH=LINUX \
    BITS=%{zsnes_bits} \
    CPU=%{zsnes_cpu} \
    WITH_SDL=yes \
    WITH_PIPEWIRE=yes \
    WITH_AO=yes

%install
install -Dm0755 zsnes %{buildroot}%{_bindir}/zsnes

for icon_size in 16x16 32x32 48x48 64x64 128x128; do
    install -Dm0644 "img/${icon_size}x32.png" \
        "%{buildroot}%{_datadir}/icons/hicolor/${icon_size}/apps/io.github.xyproto.zsnes.png"
done

install -d %{buildroot}%{_datadir}/applications
sed 's|^Exec=zsnes |Exec=%{_bindir}/zsnes |' linux/zsnes.desktop \
    > %{buildroot}%{_datadir}/applications/io.github.xyproto.zsnes.desktop
install -m0644 \
    %{SOURCE1} \
    %{SOURCE2} \
    %{SOURCE3} \
    %{buildroot}%{_datadir}/applications/

install -Dm0644 linux/io.github.xyproto.zsnes.metainfo.xml \
    %{buildroot}%{_datadir}/metainfo/io.github.xyproto.zsnes.metainfo.xml
install -Dm0644 man/zsnes.1 %{buildroot}%{_mandir}/man1/zsnes.1

%check
%make_build test \
    ARCH=LINUX \
    BITS=%{zsnes_bits} \
    CPU=%{zsnes_cpu} \
    WITH_SDL=yes \
    WITH_PIPEWIRE=yes \
    WITH_AO=yes

# Verify the installed executable's ABI directly from the ELF header.
elf_header="$(readelf -hW %{buildroot}%{_bindir}/zsnes)"
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

if ldd %{buildroot}%{_bindir}/zsnes | grep -q 'not found'; then
    echo 'Unresolved runtime dependency:' >&2
    ldd %{buildroot}%{_bindir}/zsnes >&2
    exit 1
fi

needed="$(readelf -dW %{buildroot}%{_bindir}/zsnes)"
for lib in libSDL3.so.0 libpipewire-0.3.so.0 libao.so.4; do
    if ! printf '%s\n' "$needed" | grep -Fq "Shared library: [$lib]"; then
        echo "Missing expected audio library: $lib" >&2
        exit 1
    fi
done

applications=%{buildroot}%{_datadir}/applications
main="$applications/io.github.xyproto.zsnes.desktop"
desktop-file-validate "$main"
grep -Fxq 'Name=ZSNES' "$main"
grep -Fxq 'Comment=Super Nintendo emulator' "$main"
grep -Fxq 'Exec=%{_bindir}/zsnes %f' "$main"
grep -Fxq 'TryExec=zsnes' "$main"
grep -Fxq 'Icon=io.github.xyproto.zsnes' "$main"
grep -q '^MimeType=' "$main"

check_launcher() {
    launcher="$applications/io.github.xyproto.zsnes.$1.desktop"
    desktop-file-validate "$launcher"
    grep -Fxq "Name=ZSNES ($2)" "$launcher"
    grep -Fxq "Exec=$3" "$launcher"
    grep -Fxq 'TryExec=zsnes' "$launcher"
    grep -Fxq 'Icon=io.github.xyproto.zsnes' "$launcher"
    if grep -q '^MimeType=' "$launcher"; then
        echo "Only the default launcher should register ROM file types: $launcher" >&2
        exit 1
    fi
}

check_launcher pipewire 'PipeWire' '%{_bindir}/zsnes -ad pipewire %f'
check_launcher sdl3 'SDL3' '%{_bindir}/zsnes -ad sdl %f'
check_launcher libao 'libao' '%{_bindir}/zsnes -ad ao %f'
grep -q '<id>io.github.xyproto.zsnes</id>' \
    %{buildroot}%{_datadir}/metainfo/io.github.xyproto.zsnes.metainfo.xml
grep -q '<binary>zsnes</binary>' \
    %{buildroot}%{_datadir}/metainfo/io.github.xyproto.zsnes.metainfo.xml

%files
%license COPYING miniz-LICENSE
%doc README.md
%{_bindir}/zsnes
%{_datadir}/applications/io.github.xyproto.zsnes.desktop
%{_datadir}/applications/io.github.xyproto.zsnes.pipewire.desktop
%{_datadir}/applications/io.github.xyproto.zsnes.sdl3.desktop
%{_datadir}/applications/io.github.xyproto.zsnes.libao.desktop
%{_datadir}/metainfo/io.github.xyproto.zsnes.metainfo.xml
%{_datadir}/icons/hicolor/16x16/apps/io.github.xyproto.zsnes.png
%{_datadir}/icons/hicolor/32x32/apps/io.github.xyproto.zsnes.png
%{_datadir}/icons/hicolor/48x48/apps/io.github.xyproto.zsnes.png
%{_datadir}/icons/hicolor/64x64/apps/io.github.xyproto.zsnes.png
%{_datadir}/icons/hicolor/128x128/apps/io.github.xyproto.zsnes.png
%{_mandir}/man1/zsnes.1*

%changelog
* Wed Nov 25 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.3.9-2
- Align package, executable and desktop identifiers with upstream.

* Mon Nov 23 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.3.9-1
- Update to upstream 2.3.9 and retain Fedora PIE hardening on x86_64.
- Record the bundled miniz library and ship its license notices.

* Mon Nov 23 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.3.4-2
- Enable SDL3, native PipeWire and libao audio.
- Default the desktop launcher to libao via PulseAudio.
- Add explicit SDL3/PulseAudio and native PipeWire launchers.

* Sun Sep 27 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.3.4-1
- Update to ZSNES 2 2.3.4.
- Adapt the package to upstream's C11 multi-architecture Linux build interface.
- Remove the obsolete NASM and i686-only packaging assumptions.
- Update icon installation for the upstream img/ directory layout.
- Run the upstream test suite, including its 32-bit x86 tests on x86_64.
- Disable Fedora's PIE injection only on x86 to match upstream's non-PIE ABI.

* Mon Sep 07 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.2.3-1
- Initial Fedora package.
- Package ZSNES 2 separately as zsnes2 so it can coexist with historical zsnes.
- Build the supported 32-bit x86 target with SDL3 and OpenGL support.
- Allow the required non-PIC text relocations on Fedora 44 i686.
