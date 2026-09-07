Name:           zsnes2
Version:        2.2.3
Release:        1%{?dist}
Summary:        ZSNES 2 Super Nintendo emulator

License:        GPL-2.0-only AND LGPL-2.1-or-later AND Zlib
URL:            https://github.com/xyproto/zsnes
Source0:        %{name}-%{version}.tar.gz

# The Linux port intentionally remains a 32-bit x86 program.  The remaining
# assembly implementation is ELF32/i386 code and upstream builds with -m32.
ExclusiveArch:  i686

BuildRequires:  binutils
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  nasm
BuildRequires:  python3
BuildRequires:  pkgconf-pkg-config
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(sdl3)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  desktop-file-utils

%description
ZSNES 2 is a maintained continuation of the classic ZSNES emulator,
updated to build and run on modern Unix-like systems while retaining its
low-overhead x86 implementation and familiar interface.

The Linux build is a 32-bit x86 program and provides both SDL software
rendering and OpenGL rendering. Game ROM images are not included.

%prep
%autosetup -n %{name}-%{version}

%build
# Preserve Fedora's normal hardening/compiler flags where compatible. Upstream
# deliberately appends -m32, -fno-pic and -no-pie because this is a legacy
# 32-bit x86/assembly program and cannot be built as PIE.  Fedora 44's i686
# linker otherwise rejects the resulting required dynamic text relocations, so
# relax only that linker check while retaining the rest of Fedora's flags.
%set_build_flags
%make_build \
    ARCH=LINUX \
    BINARY=zsnes2 \
    EXTRA_CFLAGS='-DZCONF=\"zsnes2\"' \
    EXTRA_LDFLAGS='-Wl,-z,notext' \
    WITH_PIPEWIRE= \
    WITH_AO=

%install
install -Dm0755 zsnes2 %{buildroot}%{_bindir}/zsnes2

for icon_size in 16x16 32x32 48x48 64x64 128x128; do
    install -Dm0644 "icons/${icon_size}x32.png" \
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
    BINARY=zsnes2 \
    EXTRA_CFLAGS='-DZCONF=\"zsnes2\"' \
    EXTRA_LDFLAGS='-Wl,-z,notext' \
    WITH_PIPEWIRE= \
    WITH_AO=

# Verify the actual ELF header instead of parsing file(1)'s human-readable
# description, whose wording is not an ABI and varies between file releases.
readelf -hW %{buildroot}%{_bindir}/zsnes2 | grep -Eq 'Class:[[:space:]]+ELF32'
readelf -hW %{buildroot}%{_bindir}/zsnes2 | grep -Eq 'Machine:[[:space:]]+Intel 80386'

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
* Mon Sep 07 2026 Reaper <JohnGrimmReaper@disroot.org> - 2.2.3-1
- Initial Fedora package.
- Package ZSNES 2 separately as zsnes2 so it can coexist with historical zsnes.
- Build the supported 32-bit x86 target with SDL3 and OpenGL support.
- Allow the required non-PIC text relocations on Fedora 44 i686.
