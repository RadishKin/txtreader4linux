Summary:  	Txt Reader(文本阅读器) for linux
Name:           txtreader
Version:        0.5.2
Release:        1
License:       	GPL
Group:          Desktop/KDE
BuildRoot:     	/var/tmp/%{name}-%{version}-root
Source:        %{name}-%{version}.tar.gz
%description
Txt Reader(文本阅读器) for linux 0.5.2
%prep
%setup
%Build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/share/txtreader
cp -f txtreader $RPM_BUILD_ROOT/usr/share/txtreader/
cp -f images/book.png $RPM_BUILD_ROOT/usr/share/txtreader/
cp -f README_*.HTML $RPM_BUILD_ROOT/usr/share/txtreader/
#Install application link for X-Windows
mkdir -p $RPM_BUILD_ROOT/usr/share/applications
cat > cat > $RPM_BUILD_ROOT/usr/share/applications/txtreader.desktop <<EOF
[Desktop Entry]
Name=Txt Reader
GenericName[zh_CN]=文本阅读器
Comment[zh_CN]=Txt Reader(文本阅读器) for linux
Comment=%{summary}
Exec=/usr/share/txtreader/txtreader
Icon=/usr/share/txtreader/book.png
Terminal=0
Type=Application
Categories=Qt;KDE;Utility;
EOF
rpmclean
%clean
rm -rf $RPM_BUILD_ROOT
%post
%files
/usr
%changelog
* Wed Jun 24 2009 lsyer <lishao378@sohu.com>
- create
