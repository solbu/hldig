INCLUDES = -I$(top_srcdir)/../htlib

AUTOMAKE_OPTIONS = foreign no-dependencies

EXTRA_DIST = db_config.h.in db.h.in \
	68K.gcc alpha.gcc sparc.gcc x86.gcc \
	hash.src log.src qam.src txn.src prefix-symbols.sh \
	LICENSE

pkglib_LTLIBRARIES = libhtdb.la

libhtdb_la_SOURCES = \
	bt_compare.c \
	bt_conv.c \
	bt_curadj.c \
	bt_cursor.c \
	bt_delete.c \
	bt_method.c \
	bt_open.c \
	bt_put.c \
	bt_rec.c \
	bt_reclaim.c \
	bt_recno.c \
	bt_rsearch.c \
	bt_search.c \
	bt_split.c \
	bt_stat.c \
	bt_upgrade.c \
	btree_auto.c \
	crdel_auto.c \
	crdel_rec.c \
	db.c \
	db_am.c \
	db_auto.c \
	db_byteorder.c \
	db_conv.c \
	db_dispatch.c \
	db_dup.c \
	db_err.c \
	db_getlong.c \
	db_iface.c \
	db_join.c \
	db_log2.c \
	db_meta.c \
	db_method.c \
	db_overflow.c \
	db_pr.c \
	db_rec.c \
	db_reclaim.c \
	db_ret.c \
	db_salloc.c \
	db_shash.c \
	db_upgrade.c \
	env_method.c \
	env_open.c \
	env_recover.c \
	env_region.c \
	hash.c \
	hash_auto.c \
	hash_conv.c \
	hash_dup.c \
	hash_func.c \
	hash_meta.c \
	hash_method.c \
	hash_page.c \
	hash_rec.c \
	hash_reclaim.c \
	hash_stat.c \
	hash_upgrade.c \
	lock.c \
	lock_conflict.c \
	lock_deadlock.c \
	lock_region.c \
	lock_util.c \
	log.c \
	log_archive.c \
	log_auto.c \
	log_compare.c \
	log_findckp.c \
	log_get.c \
	log_method.c \
	log_put.c \
	log_rec.c \
	log_register.c \
	mp_alloc.c \
	mp_bh.c \
	mp_cmpr.c \
	mp_fget.c \
	mp_fopen.c \
	mp_fput.c \
	mp_fset.c \
	mp_method.c \
	mp_region.c \
	mp_register.c \
	mp_stat.c \
	mp_sync.c \
	mp_trickle.c \
	mut_fcntl.c \
	mut_pthread.c \
	mut_tas.c \
	mutex.c \
	os_abs.c \
	os_alloc.c \
	os_dir.c \
	os_errno.c \
	os_fid.c \
	os_finit.c \
	os_fsync.c \
	os_handle.c \
	os_map.c \
	os_method.c \
	os_oflags.c \
	os_open.c \
	os_region.c \
	os_rename.c \
	os_root.c \
	os_rpath.c \
	os_rw.c \
	os_seek.c \
	os_sleep.c \
	os_spin.c \
	os_stat.c \
	os_tmpdir.c \
	os_unlink.c \
	qam.c \
	qam_auto.c \
	qam_conv.c \
	qam_method.c \
	qam_open.c \
	qam_rec.c \
	qam_stat.c \
	txn.c \
	txn_auto.c \
	txn_rec.c \
	txn_region.c \
	xa.c \
	xa_db.c \
	xa_map.c

libhtdb_la_LDFLAGS = -release $(MAJOR_VERSION).$(MINOR_VERSION).$(MICRO_VERSION)

pkginclude_HEADERS = \
	btree.h \
	btree_auto.h \
	btree_ext.h \
	common_ext.h \
	crdel_auto.h \
	db.h \
	db_am.h \
	db_auto.h \
	db_config.h \
	db_dispatch.h \
	db_ext.h \
	db_int.h \
	db_join.h \
	db_page.h \
	db_shash.h \
	db_swap.h \
	debug.h \
	env_ext.h \
	hash.h \
	hash_auto.h \
	hash_ext.h \
	lock.h \
	lock_ext.h \
	log.h \
	log_auto.h \
	log_ext.h \
	mp.h \
	mp_ext.h \
	mutex.h \
	mutex_ext.h \
	os.h \
	os_ext.h \
	os_jump.h \
	qam.h \
	qam_auto.h \
	qam_ext.h \
	queue.h \
	region.h \
	shqueue.h \
	txn.h \
	txn_auto.h \
	txn_ext.h \
	xa.h \
	xa_ext.h
