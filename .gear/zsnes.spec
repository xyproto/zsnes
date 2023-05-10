%define _unpackaged_files_terminate_build 1

Name: zsnes
Version: 2.0.12
Release: alt1
Summary: Super Nintendo emulator
Group: Emulators
License: GPLv2
URL: https://github.com/xyproto/zsnes
Source0: %name-%version.tar
BuildArch: x86_64

BuildRequires: libSDL-devel
BuildRequires: libpng-devel
BuildRequires: nasm
BuildRequires: gcc-c++

%description
ZSNES is Super Nintendo emulator. It works with 64-bit x86

%prep
%setup -q

%build
%make

%install
%makeinstall_std

%files
%_bindir/%name
%_datadir/metainfo/*%name.metainfo.xml
%_desktopdir/*%name.desktop
%_iconsdir/hicolor/*/apps/*%name.png
%_man1dir/%name.1*

%changelog