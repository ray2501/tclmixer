%{!?directory:%define directory /usr}

%define buildroot %{_tmppath}/%{name}

Name:          tclmixer
Summary:       SDL_mixer (SDL) bindings for Tcl
Version:       2.0.2
Release:       0
License:       LGPL-2.1-only
Group:         Development/Libraries/Tcl
Source:        %{name}-%{version}.tar.gz
URL:           https://github.com/ray2501/tclmixer
BuildRequires: autoconf
BuildRequires: make
BuildRequires: tcl-devel >= 8.4
BuildRequires: libSDL2-devel
BuildRequires: libSDL2_mixer-devel
Requires:      tcl >= 8.4
BuildRoot:     %{buildroot}

%description
TclMixer provides SDL_mixer (SDL) bindings for Tcl.

It allows to play multiple sounds simultaneously using a built-in
software mixer.

%prep
%setup -q -n %{name}-%{version}

%build
./configure \
	--prefix=%{directory} \
	--exec-prefix=%{directory} \
	--libdir=%{directory}/%{_lib}
make 

%install
make DESTDIR=%{buildroot} pkglibdir=%{tcl_archdir}/%{name}%{version} install

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%{tcl_archdir}
