
include $(top_srcdir)/Makefile.config

EXTRA_DIST = COPYING  \
	ChangeLog \
	FAQ.html \
	RELEASE.html \
	THANKS.html \
	TODO.html \
	author.html \
	bdot.gif \
	bugs.html \
	attrs.html attrs_head.html attrs_tail.html \
	cf_byname.html cf_byname_head.html cf_byname_tail.html \
	cf_byprog.html cf_byprog_head.html cf_byprog_tail.html \
	cf_general.html \
	cf_types.html \
	cf_variables.html \
	config.html \
	confindex.html \
	confmenu.html \
	contents.html \
	dot.gif \
	htdig.gif \
	htdig.html \
	htdig_big.gif \
	htfuzzy.html \
	htmerge.html \
	htnotify.html \
	hts_form.html \
	hts_general.html \
	hts_menu.html \
	hts_method.html \
	hts_templates.html \
	htsearch.html \
	index.html \
	install.html \
	ma_menu.html \
	mailarchive.html \
	mailing.html \
	main.html \
	meta.html \
	notification.html \
	require.html \
	triangle.gif \
	up.gif \
	uses.html \
	where.html \
	cf_generate.pl

attrs.html.in: attrs_head.html attrs_tail.html ../htcommon/defaults.cc cf_generate.pl
	@PERL@ ${srcdir}/cf_generate.pl ${top_srcdir}

cf_byname.html: cf_byname_head.html cf_byname_tail.html ../htcommon/defaults.cc cf_generate.pl
	@PERL@ ${srcdir}/cf_generate.pl ${top_srcdir}

cf_byprog.html: cf_byprog_head.html cf_byprog_tail.html ../htcommon/defaults.cc cf_generate.pl
	@PERL@ ${srcdir}/cf_generate.pl ${top_srcdir}
