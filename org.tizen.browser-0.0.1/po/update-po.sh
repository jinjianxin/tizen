#!/bin/sh

PACKAGE=browser
SRCROOT=..
POTFILES=POTFILES.in

#ALL_LINGUAS= am az be ca cs da de_DE el_GR en es_ES et fi fr_FR hr hu it_IT ja_JP ko_KR lv mk ml ms nb ne nl_NL pa pl pt_PT pt_BR ru_RU rw sk sl sr sr@Latn sv ta tr_TR uk vi zh_CN zh_TW
ALL_LINGUAS="en.po en_PH.po en_US.po ar.po az.po bg.po ca.po cs.po da.po de_DE.po el_GR.po es_ES.po es_US.po et.po eu.po fi.po fr_CA.po fr_FR.po ga.po gl.po hi.po hr.po hu.po hy.po is.po it_IT.po ja_JP.po ka.po kk.po ko_KR.po lt.po lv.po mk.po nb.po nl_NL.po pl.po pt_BR.po pt_PT.po ro.po ru_RU.po sk.po sl.po sr.po sv.po tr_TR.po uk.po uz.po zh_CN.po zh_HK.po zh_SG.po zh_TW.po"

XGETTEXT=/usr/bin/xgettext
MSGMERGE=/usr/bin/msgmerge

echo -n "Make ${PACKAGE}.pot  "
if [ ! -e $POTFILES ] ; then
	echo "$POTFILES not found"
	exit 1
fi

$XGETTEXT --default-domain=${PACKAGE} --directory=${SRCROOT} \
		--add-comments --keyword=_ --keyword=N_ --files-from=$POTFILES \
&& test ! -f ${PACKAGE}.po \
	|| (rm -f ${PACKAGE}.pot && mv ${PACKAGE}.po ${PACKAGE}.pot)

if [ $? -ne 0 ]; then
	echo "error"
	exit 1
else
	echo "done"
fi

for LANG in $ALL_LINGUAS; do 
	echo "$LANG : "

	if [ ! -e $LANG.po ] ; then
		sed 's/CHARSET/UTF-8/g' ${PACKAGE}.pot > ${LANG}.po
		echo "${LANG}.po created"
	else
		if $MSGMERGE ${LANG}.po ${PACKAGE}.pot -o ${LANG}.new.po ; then
			if cmp ${LANG}.po ${LANG}.new.po > /dev/null 2>&1; then
				rm -f ${LANG}.new.po
			else
				if mv -f ${LANG}.new.po ${LANG}.po; then
					echo ""	
				else
					echo "msgmerge for $LANG.po failed: cannot move $LANG.new.po to $LANG.po" 1>&2
					rm -f ${LANG}.new.po
					exit 1
				fi
			fi
		else
			echo "msgmerge for $LANG failed!"
			rm -f ${LANG}.new.po
		fi
	fi
	echo ""
done

