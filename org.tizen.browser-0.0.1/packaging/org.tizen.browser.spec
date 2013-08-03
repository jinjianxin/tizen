%define appdir /opt/apps/org.tizen.browser

Name:       org.tizen.browser
Summary:    webkit browser with EFL
Version: 0.0.1
Release:    1
Group:      misc
License:    TO_BE_FILLED
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/sqlite3

BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(secure-storage)
BuildRequires:  pkgconfig(libsoup-2.4)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libssl)
BuildRequires:  pkgconfig(capi-location-manager)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(ewebkit2)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(devman)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-web-url-download)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(capi-system-haptic)
BuildRequires:  pkgconfig(shortcut)

BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(gnutls)
BuildRequires:  pkgconfig(embryo)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(vconf-internal-keys)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-sensor)


BuildRequires:  cmake
BuildRequires:  gettext
BuildRequires:  edje-tools

%description
webkit browser with EFL.

%prep
%setup -q

%build
CFLAGS+=" -fPIC";export CFLAGS
CXXFLAGS+=" -fPIC";export CXXFLAGS
export LDFLAGS+="-Wl,--rpath=%{appdir}/lib -Wl,--hash-style=both -Wl,--as-needed"
LDFLAGS="$LDFLAGS"
cmake . -DCMAKE_INSTALL_PREFIX=%{appdir}

make %{?jobs:-j%jobs}

%install
%make_install

%post
mkdir -p /opt/dbspace/
##### History ######
if [ ! -f /opt/dbspace/.browser-history.db ];
then
	sqlite3 /opt/dbspace/.browser-history.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE history(id INTEGER PRIMARY KEY AUTOINCREMENT, address, title, counter INTEGER, visitdate DATETIME, favicon BLOB, favicon_length INTEGER, favicon_w INTEGER, favicon_h INTEGER);'
fi

### Bookmark ###
if [ ! -f /opt/dbspace/.internet_bookmark.db ];
then
	sqlite3 /opt/dbspace/.internet_bookmark.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE bookmarks(id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, parent INTEGER, address, title, creationdate, sequence INTEGER, updatedate, visitdate, editable INTEGER, accesscount INTEGER, favicon BLOB, favicon_length INTEGER, favicon_w INTEGER, favicon_h INTEGER);
	create index idx_bookmarks_on_parent_type on bookmarks(parent, type);

	insert into bookmarks (type, parent, title, creationdate, editable, sequence, accesscount) values(1, 0, "Bookmarks", DATETIME("now"),  0, 1, 0);'
fi

mkdir -p /opt/apps/org.tizen.browser/data/db/

##### Geolocation ######
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-geolocation.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-geolocation.db 'PRAGMA journal_mode=PERSIST;
	create table geolocation(id integer primary key autoincrement, address, accept INTEGER,updatedate DATETIME);'
fi

##### Password ######
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-credential.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-credential.db 'PRAGMA journal_mode=PERSIST;
	create table passwords(id integer primary key autoincrement, address, login, password)'
fi

##### Most visited ######
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db 'PRAGMA journal_mode=PERSIST;
	create table mostvisited(id integer primary key, address, title, image)'

	#default Sites
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db 'PRAGMA journal_mode=PERSIST;
	insert into mostvisited values(0, "http://mobile.twitter.com/", "Twitter", "default_0");
	insert into mostvisited values(1, "http://m.facebook.com/", "Facebook", "default_1");
	insert into mostvisited values(2, "http://m.naver.com/", "Naver", "default_2");
	insert into mostvisited values(3, "http://m.nate.com/", "Nate", "default_3")'
fi

#### USER AGENTS #####
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
	create table user_agents(name primary key, value)'
	# mobile
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
	insert into user_agents values("Galaxy S", "Mozilla/5.0 (Linux; U; Android 2.3.7; en-gb; GT-I9000 Build/GRJ22) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("Galaxy S II", "Mozilla/5.0 (Linux; U; Android 2.3.5; en-gb; GT-I9100 Build/GINGERBREAD) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("Galaxy S III", "Mozilla/5.0 (Linux; U; Android 4.0.3; en-gb; GT-I9300 Build/IML74K) AppleWebkit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30");
	insert into user_agents values("SLP Galaxy", "Mozilla/5.0 (Linux; U; Android 2.3.4; en-us; GT-I9500 Build/GINGERBREAD) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("Tizen", "Mozilla/5.0 (Linux; U; Tizen 1.0; en-us) AppleWebKit/534.46 (KHTML, like Gecko) Mobile Tizen Browser/1.0");
	insert into user_agents values("Galaxy Nexus", "Mozilla/5.0 (Linux; U; Android 4.0.3; en-us; Galaxy Nexus Build/IML74K) AppleWebKit/535.7 (KHTML, like Gecko) Mobile Safari/535.7");
	insert into user_agents values("Samsung", "Mozilla/5.0 (SAMSUNG; SAMSUNG-GT-I9200/1.0; U; Linux/SLP/2.0; ko-kr) AppleWebKit/534.4 (KHTML, like Gecko) Dolfin/2.0 Mobile");
	insert into user_agents values("Samsung Dolfin", "SAMSUNG-GT-S8500/S8500XXJD2 SHP/VPP/R5 Dolfin/2.0 Nextreaming SMM-MMS/1.2.0 profile/MIDP-2.1 configuration/CLDC-1.1");
	insert into user_agents values("Apple iPhone 3", "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_1_3 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7E18 Safari/528.16");
	insert into user_agents values("Apple iPhone 4", "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_5 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8L1 Safari/6533.18.5");
	insert into user_agents values("Apple iOS 5", "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0_1 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A405 Safari/7534.48.3");
	insert into user_agents values("Android 2.3 (Nexus One)", "Mozilla/5.0 (Linux; U; Android 2.3.4; en-us; Nexus One Build/GRJ22) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("Opera Mobi", "Opera/9.80 (Windows NT 6.1; U; Edition IBIS; en) Presto/2.6.30 Version/10.63");
	insert into user_agents values("Samsung Bada", "Mozilla/5.0 (SAMSUNG; SAMSUNG-GT-S8500/1.0; U; Bada/1.0; en-us) AppleWebKit/533.1 (KHTML, like Gecko) Dolfin/2.0 Mobile WVGA SMM-MMS/1.2.0 OPN-B");
	insert into user_agents values("Orange TV", "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3 OrangeAppliTV/2.3.9");
	insert into user_agents values("Chrome Browser for android", "Mozilla/5.0 (Linux; U; Android 4.0.1; ko-kr; Galaxy Nexus Build/ITL41F) AppleWebKit/535.7 (KHTML, like Gecko) CrMo/16.0.912.75 Mobile Safari/535.7");
	insert into user_agents values("MANGO(Nokia 800C)", "Mozilla/5.0 (compatible; MSIE 9.0; Windows Phone OS 7.5; Trident/5.0; IEMobile/9.0; Nokia; 800C)");
	insert into user_agents values("System user agent", "");
	insert into user_agents values("Samsung Bada 2.0", "Mozilla/5.0 (SAMSUNG; SAMSUNG-GT-S8500/1.0; U; Bada/2.0; en-us) AppleWebKit/534.20 (KHTML, like Gecko) Mobile WVGA SMM-MMS/1.2.0 OPN-B Dolfin/3.0")'

	# desktop
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
	insert into user_agents values("Samsung Desktop", "Mozilla/5.0 (U; Linux/SLP/2.0; ko-kr) AppleWebKit/533.1 (KHTML, like Gecko)");
	insert into user_agents values("Firefox 5", "Mozilla/5.0 (Windows NT 6.1; rv:9.0.1) Gecko/20100101 Firefox/9.0.1");
	insert into user_agents values("Firefox 5 Fennec(Mobile)", "Mozilla/5.0 (Android; Linux armv7l; rv:5.0) Gecko/20110615 Firefox/5.0 Fennec/5.0");
	insert into user_agents values("Safari 5.0", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_2) AppleWebKit/534.52.7 (KHTML, like Gecko) Version/5.1.2 Safari/534.52.7");
	insert into user_agents values("Google Chrome 18.0", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.16 (KHTML, like Gecko) Chrome/18.0.1003.1 Safari/535.16");
	insert into user_agents values("Internet Explorer 9", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");
	insert into user_agents values("Galaxy Tab 10.1", "Mozilla/5.0 (Linux; U; Android 3.0.1; en-us; GT-P7100 Build/HRI83) AppleWebKit/534.13 (KHTML, like Gecko) Version/4.0 Safari/534.13");
	insert into user_agents values("iPad 2", "Mozilla/5.0 (iPad; U; CPU OS 4_3_5 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8L1 Safari/6533.18.5")'
fi


# create cookies db
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-cookies.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db 'CREATE TABLE moz_cookies (id INTEGER PRIMARY KEY, name TEXT, value TEXT, host TEXT, path TEXT,expiry INTEGER, lastAccessed INTEGER, isSecure INTEGER, isHttpOnly INTEGER);'
	#test cookie
	#sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db 'INSERT INTO moz_cookies values(NULL, "cookies_name_test", "cookies_value_test", "www.cookies_test.com", "cookies_path_test", 2011, NULL, 1, 0);'
fi

# Change db file owner & permission
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser.db
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser.db-journal
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db-journal
chown :5000 /opt/dbspace/.browser-history.db
chown :5000 /opt/dbspace/.browser-history.db-journal
chown :5000 /opt/dbspace/.internet_bookmark.db
chown :5000 /opt/dbspace/.internet_bookmark.db-journal
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-credential.db
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-credential.db-journal
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db-journal
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-geolocation.db
#chown :5000 /opt/apps/org.tizen.browser/data/db/.browser-geolocation.db-journal
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser.db
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser.db-journal
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db-journal
chmod 666 /opt/dbspace/.browser-history.db
chmod 666 /opt/dbspace/.browser-history.db-journal
chmod 666 /opt/dbspace/.internet_bookmark.db
chmod 666 /opt/dbspace/.internet_bookmark.db-journal
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-credential.db
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-credential.db-journal
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-mostvisited.db-journal
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-geolocation.db
chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-geolocation.db-journal

##################################################
# set default vconf values
##################################################
vconftool set -t string db/browser/browser_user_agent "System user agent" -g 5000 -f
vconftool set -t string db/browser/custom_user_agent "" -g 5000 -f

# Change file owner
chown -R 5000:5000 /opt/apps/org.tizen.browser/data

%files
%defattr(-,root,root,-)
/opt/apps/org.tizen.browser/bin/browser
/opt/apps/org.tizen.browser/data/screenshots/default_*
/opt/apps/org.tizen.browser/res/edje/*.edj
/opt/apps/org.tizen.browser/res/html/*
/opt/share/icons/default/small/org.tizen.browser.png
/opt/apps/org.tizen.browser/res/images/*.png
/opt/apps/org.tizen.browser/res/locale/*/*/browser.mo
/opt/apps/org.tizen.browser/data/default_application_icon.png
/opt/apps/org.tizen.browser/data/config_sample.xml
/opt/apps/org.tizen.browser/data/xml/
/opt/share/packages/*
