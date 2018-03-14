ACLOCAL_AMFLAGS = -I build-aux/m4
SUBDIRS = src
if ENABLE_MAN
SUBDIRS += doc/man
endif
.PHONY: deploy FORCE

override GZIP_ENV="-9n"

if BUILD_BITCOIN_LIBS
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libzcashconsensus.pc
endif

BITCOIND_BIN=$(top_builddir)/src/$(BITCOIN_DAEMON_NAME)$(EXEEXT)
BITCOIN_CLI_BIN=$(top_builddir)/src/$(BITCOIN_CLI_NAME)$(EXEEXT)

DIST_DOCS = $(wildcard doc/*.md) $(wildcard doc/release-notes/*.md)

BIN_CHECKS=$(top_srcdir)/contrib/devtools/symbol-check.py \
           $(top_srcdir)/contrib/devtools/security-check.py

COVERAGE_INFO = baseline_filtered_combined.info baseline.info block_test.info \
  leveldb_baseline.info test_bitcoin_filtered.info total_coverage.info \
  baseline_filtered.info block_test_filtered.info \
  leveldb_baseline_filtered.info test_bitcoin_coverage.info test_bitcoin.info \
  litecoinz-gtest.info litecoinz-gtest_filtered.info litecoinz-gtest_coverage.info

dist-hook:
	-$(MAKE) -C $(top_distdir)/src/leveldb clean
	-$(MAKE) -C $(top_distdir)/src/secp256k1 distclean
	-$(GIT) archive --format=tar HEAD -- src/clientversion.cpp | $(AMTAR) -C $(top_distdir) -xf -

distcheck-hook:
	$(MKDIR_P) $(top_distdir)/_build/src/leveldb
	cp -rf $(top_srcdir)/src/leveldb/* $(top_distdir)/_build/src/leveldb/
	-$(MAKE) -C $(top_distdir)/_build/src/leveldb clean

distcleancheck:
	@:

$(if $(findstring src/,$(MAKECMDGOALS)),$(MAKECMDGOALS), none): FORCE
	$(MAKE) -C src $(patsubst src/%,%,$@)

$(BITCOIND_BIN): FORCE
	$(MAKE) -C src $(@F)

$(BITCOIN_CLI_BIN): FORCE
	$(MAKE) -C src $(@F)

if USE_LCOV

baseline.info:
	$(LCOV) -c -i -d $(abs_builddir)/src -o $@

baseline_filtered.info: baseline.info
	$(LCOV) -r $< "/usr/include/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/*.h" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/boost/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gmock/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gtest/*" \
                      "$(abs_builddir)/src/gtest/*" \
                      "$(abs_builddir)/src/test/*" \
                      "$(abs_builddir)/src/wallet/gtest/*" \
                      "$(abs_builddir)/src/wallet/test/*" \
                      -o $@

leveldb_baseline.info: baseline_filtered.info
	$(LCOV) -c -i -d $(abs_builddir)/src/leveldb -b $(abs_builddir)/src/leveldb -o $@

leveldb_baseline_filtered.info: leveldb_baseline.info
	$(LCOV) -r $< "/usr/include/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/*.h" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/boost/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gmock/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gtest/*" \
                      "$(abs_builddir)/src/gtest/*" \
                      "$(abs_builddir)/src/test/*" \
                      "$(abs_builddir)/src/wallet/gtest/*" \
                      "$(abs_builddir)/src/wallet/test/*" \
                      -o $@

baseline_filtered_combined.info: leveldb_baseline_filtered.info baseline_filtered.info
	$(LCOV) -a leveldb_baseline_filtered.info -a baseline_filtered.info -o $@

test_bitcoin.info: baseline_filtered_combined.info
	$(MAKE) -C src/ bitcoin_test_check
	$(LCOV) -c -d $(abs_builddir)/src -t test_bitcoin -o $@
	$(LCOV) -z -d $(abs_builddir)/src
	$(LCOV) -z -d $(abs_builddir)/src/leveldb

test_bitcoin_filtered.info: test_bitcoin.info
	$(LCOV) -r $< "/usr/include/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/*.h" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/boost/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gmock/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gtest/*" \
                      "$(abs_builddir)/src/gtest/*" \
                      "$(abs_builddir)/src/test/*" \
                      "$(abs_builddir)/src/wallet/gtest/*" \
                      "$(abs_builddir)/src/wallet/test/*" \
                      -o $@

litecoinz-gtest.info: baseline_filtered_combined.info
	$(MAKE) -C src/ litecoinz-gtest_check
	$(LCOV) -c -d $(abs_builddir)/src -t litecoinz-gtest -o $@
	$(LCOV) -z -d $(abs_builddir)/src
	$(LCOV) -z -d $(abs_builddir)/src/leveldb

litecoinz-gtest_filtered.info: litecoinz-gtest.info
	$(LCOV) -r $< "/usr/include/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/*.h" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/boost/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gmock/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gtest/*" \
                      "$(abs_builddir)/src/gtest/*" \
                      "$(abs_builddir)/src/test/*" \
                      "$(abs_builddir)/src/wallet/gtest/*" \
                      "$(abs_builddir)/src/wallet/test/*" \
                      -o $@

block_test.info: test_bitcoin_filtered.info
	$(MKDIR_P) qa/tmp
	-@TIMEOUT=15 qa/pull-tester/run-litecoinzd-for-test.sh $(JAVA) -jar $(JAVA_COMPARISON_TOOL) qa/tmp/compTool 0
	$(LCOV) -c -d $(abs_builddir)/src --t BitcoinJBlockTest -o $@
	$(LCOV) -z -d $(abs_builddir)/src
	$(LCOV) -z -d $(abs_builddir)/src/leveldb

block_test_filtered.info: block_test.info
	$(LCOV) -r $< "/usr/include/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/*.h" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/boost/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gmock/*" \
                      "$(abs_builddir)/depends/x86_64-unknown-linux-gnu/include/gtest/*" \
                      "$(abs_builddir)/src/gtest/*" \
                      "$(abs_builddir)/src/test/*" \
                      "$(abs_builddir)/src/wallet/gtest/*" \
                      "$(abs_builddir)/src/wallet/test/*" \
                      -o $@

test_bitcoin_coverage.info: baseline_filtered_combined.info test_bitcoin_filtered.info
	$(LCOV) -a baseline_filtered.info -a leveldb_baseline_filtered.info -a test_bitcoin_filtered.info -o $@

litecoinz-gtest_coverage.info: baseline_filtered_combined.info litecoinz-gtest_filtered.info
	$(LCOV) -a baseline_filtered.info -a leveldb_baseline_filtered.info -a litecoinz-gtest_filtered.info -o $@

total_coverage.info:  baseline_filtered_combined.info test_bitcoin_filtered.info litecoinz-gtest_filtered.info block_test_filtered.info
	$(LCOV) -a baseline_filtered.info -a leveldb_baseline_filtered.info -a test_bitcoin_filtered.info -a litecoinz-gtest_filtered.info -a block_test_filtered.info -o $@ | $(GREP) "\%" | $(AWK) '{ print substr($$3,2,50) "/" $$5 }' > coverage_percent.txt

test_bitcoin.coverage/.dirstamp:  test_bitcoin_coverage.info
	$(GENHTML) -s $< -o $(@D)
	@touch $@

litecoinz-gtest.coverage/.dirstamp:  litecoinz-gtest_coverage.info
	$(GENHTML) -s $< -o $(@D)
	@touch $@

cov-litecoinz: litecoinz-gtest.coverage/.dirstamp

total.coverage/.dirstamp: total_coverage.info
	$(GENHTML) -s $< -o $(@D)
	@touch $@

cov: test_bitcoin.coverage/.dirstamp cov-litecoinz total.coverage/.dirstamp

endif

dist_bin_SCRIPTS = zcutil/fetch-params.sh
dist_noinst_SCRIPTS = autogen.sh zcutil/build-debian-package.sh zcutil/build.sh

EXTRA_DIST = $(top_srcdir)/share/genbuild.sh qa/pull-tester/rpc-tests.sh qa/pull-tester/run-litecoinz-cli qa/rpc-tests qa/litecoinz $(DIST_DOCS) $(BIN_CHECKS)

install-exec-hook:
	mv $(DESTDIR)$(bindir)/fetch-params.sh $(DESTDIR)$(bindir)/litecoinz-fetch-params

.INTERMEDIATE: $(COVERAGE_INFO)

DISTCHECK_CONFIGURE_FLAGS = --enable-man

clean-local:
	rm -rf test_bitcoin.coverage/ litecoinz-gtest.coverage/ total.coverage/
