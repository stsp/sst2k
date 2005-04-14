Name: sst
Version: 2.0
Release: 1
URL: http://www.catb.org/~esr/sst/
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Games
Summary: Sst (screen mode)
BuildRoot: %{_tmppath}/%{name}-root
#Freshmeat-Name: strek

%description
Save the Federation from the invading Klingons!  Visit exotic planets and
strip-mine them for dilithium!  Encounter mysterious space thingies!
The classic Super Star Trek game from the days of slow teletypes, reloaded.

%prep
%setup -q

%build
make %{?_smp_mflags} all sst-doc.html

%install
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"%{_bindir}
mkdir -p "$RPM_BUILD_ROOT"%{_mandir}/man6/
cp sst "$RPM_BUILD_ROOT"%{_bindir}
cp sst.6 "$RPM_BUILD_ROOT"%{_mandir}/man6/
mkdir -p "$RPM_BUILD_ROOT"%{_defaultdocdir}/sst/
cp sst.doc "$RPM_BUILD_ROOT"%{_defaultdocdir}/sst/
cp sst-doc.html "$RPM_BUILD_ROOT"%{_defaultdocdir}/sst/index.html

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"

%files
%doc README COPYING
%defattr(-,root,root,-)
%{_mandir}/man6/sst.6*
%{_bindir}/sst
%{_defaultdocdir}/sst/sst.doc
%{_defaultdocdir}/sst/index.html

%changelog

* Thu Apr 14 2005 Eric S. Raymond <esr@snark.thyrsus.com> - 2.0-1
- First release under new management.


