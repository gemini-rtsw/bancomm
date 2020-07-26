%define _prefix /gem_base/epics/support
%define name bancomm
%define version 3.15.8
%define release 1.6.13
%define repository gemdev
%define debug_package %{nil}
%define arch %(uname -m)
%define checkout %(git log --pretty=format:'%h' -n 1) 

#These global defines are added to prevent stripping
# symbols on vxWorks cross-compiled code
# Getting 'strip' to work is probably only needed for
# building a related debug sub-package
#
# But this prevents all the strip warnings
# mrippa 20120202
%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}

Summary: %{name} Package, a module for EPICS base
Name: %{name}
Version: %{version}
Release: %release.%(date +"%Y%m%d")%{checkout}%{?dist}
License: EPICS Open License
Group: Applications/Engineering
Source0: %{name}-%{version}.tar.gz
ExclusiveArch: %{arch}
Prefix: %{_prefix}
## You may specify dependencies here
BuildRequires: epics-base-devel re2c gemini-ade psmisc
Requires: epics-base
## Switch dependency checking off
# AutoReqProv: no

%description
This is the module %{name}.

## If you want to have a devel-package to be generated uncomment the following:
%package devel
Summary: %{name}-devel Package
Group: Development/Gemini
Requires: %{name}
%description devel
This is the module %{name}.

%prep
%setup -q 

%build
#update environment from former rpm installations due to BuildRequires
source /gem_base/etc/profile
#start virtual framebuffer to have graphics for java
Xvfb :1  -ac -nolisten tcp -nolisten unix &

make distclean uninstall
DISPLAY=:1 make
killall Xvfb

%install
export DONT_STRIP=1
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/%{name}
cp -r dbd $RPM_BUILD_ROOT/%{_prefix}/%{name}
# cp -r bin $RPM_BUILD_ROOT/%{_prefix}/%{name}
cp -r lib $RPM_BUILD_ROOT/%{_prefix}/%{name}
cp -r include $RPM_BUILD_ROOT/%{_prefix}/%{name}
cp -r configure $RPM_BUILD_ROOT/%{_prefix}/%{name}
# find $RPM_BUILD_ROOT/%{_prefix}/%{name}/configure -name ".git" -exec rm -rf {} \;


%postun
if [ "$1" = "0" ]; then
	rm -rf %{_prefix}/%{name}
fi


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
#   /%{_prefix}/%{name}/bin
   /%{_prefix}/%{name}/lib

%files devel
%defattr(-,root,root)
   /%{_prefix}/%{name}/dbd
   /%{_prefix}/%{name}/include
   /%{_prefix}/%{name}/configure

%changelog
* Thu Jul 23 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-1.6.13.20200723e49c424
- new package built with tito

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.20200722f61ed7e
- finally the right Release tag (fkraemer@gemini.edu)

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.20200722.git2b87062
- new package built with tito

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.1.20200722.gite60436b
- adapted specfiles Release tag (fkraemer@gemini.edu)

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.1.20200722
- bumped specfile (fkraemer@gemini.edu)
- added sequencer.spec (fkraemer@gemini.edu)
- added BuildRequirement re2c (fkraemer@gemini.edu)

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu>
- added BuildRequirement re2c (fkraemer@gemini.edu)

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu>
- test

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.0.20200722
- changed path back and Requires tag to epics-base(-devel)
  (fkraemer@gemini.edu)
- adapted EPICS Path (fkraemer@gemini.edu)

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu>
- changed path back and Requires tag to epics-base(-devel)
  (fkraemer@gemini.edu)
- adapted EPICS Path (fkraemer@gemini.edu)

* Wed Jul 22 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.20200722
- adapted Release token (fkraemer@gemini.edu)
- corrected EPICS_BASE in config/RELEASE (fkraemer@gemini.edu)

* Fri Jul 17 2020 fkraemer <fkraemer@gemini.edu> 3.15.8-2.2.8.20200717gite6b33fb
- corrected EPICS_BASE in config/RELEASE

* Wed Jul 15 2020 Matt Rippa <mrippa@gemini.edu> 3.15.8-2.2.8.20200715gite29f3d3
- New epics path and tito releaser tests. (mrippa@gemini.edu)

* Mon Jul 13 2020 Matt Rippa <mrippa@gemini.edu> 3.15.8-2.2.8.20200713gitc7b4fa2
- Added epics-base to requires (mrippa@gemini.edu)
- Added .tito/releasers.conf (mrippa@gemini.edu)

* Fri Jul 10 2020 Matt Rippa <mrippa@gemini.edu> 3.15.8-2.2.8.20200710git5c91c94
- First successful tito build --rpm (mrippa@gemini.edu)

* Fri Jul 10 2020 Matt Rippa <mrippa@gemini.edu> 3.15.8-2.2.8.20200710gitaf3c52b
- new package built with tito

* Fri Mar 9 2012 Mathew Rippa <mrippa@gemini.edu> 2.1.13-0
- r3.14.12.2, rpmlint compliant
* Mon Feb 11 2008 Felix Kraemer <fkraemer@gemini.edu> 2.0.11-0
- changes to be compliant with EPICS build mechanism
* Wed Dec 19 2007 Felix Kraemer <fkraemer@gemini.edu> 2.0.11-0
- initial release
