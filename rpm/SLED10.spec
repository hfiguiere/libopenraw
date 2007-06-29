#
# spec file for package libopenraw
#
# Copyright (c) 2007 Novell Inc.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#

# norootforbuild


Summary: Digital camera RAW file decoding
Name: libopenraw
Version: 0.0.2
Release: 1
License: GNU Lesser General Public License (LGPL)
Group: System/Libraries
Source0: http://libopenraw.freedesktop.org/download/%name-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot
BuildRequires: boost-devel, libjpeg-devel, gtk2-devel
%define prefix   /usr

%description
A library for digital camera RAW file decoding

%prep
%setup -q


%build
CFLAGS="$RPM_OPT_FLAGS" \
    ./configure --prefix=%prefix \
    --libdir=/usr/%_lib
make

%install
rm -rf $RPM_BUILD_ROOT
DESTDIR=$RPM_BUILD_ROOT make install

%post
%run_ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README TODO COPYING ChangeLog

%{prefix}/%{_lib}/libopenraw.*
%{prefix}/%{_lib}/libopenrawgnome.*
%{prefix}/include/libopenraw-1.0/*
%{prefix}/%{_lib}/pkgconfig/*.pc

%changelog
