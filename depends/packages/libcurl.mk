package=libcurl
$(package)_version=8.4.0
$(package)_dependencies=openssl
$(package)_download_path=https://curl.haxx.se/download
$(package)_file_name=curl-$($(package)_version).tar.gz
$(package)_sha256_hash=816e41809c043ff285e8c0f06a75a1fa250211bbfb2dc0a037eeef39f1a9e427
$(package)_config_opts=--with-openssl --disable-shared --enable-static --prefix=$(host_prefix)
$(package)_config_opts_linux=--host=x86_64-unknown-linux-gnu
$(package)_config_opts_mingw32=--enable-mingw --host=x86_64-w64-mingw32
$(package)_cflags_darwin=-mmacosx-version-min=$(OSX_MIN_VERSION)
$(package)_conf_tool=./configure

ifeq ($(build_os),darwin)
define $(package)_set_vars
  $(package)_build_env=MACOSX_DEPLOYMENT_TARGET="$(OSX_MIN_VERSION)"
endef
endif

ifeq ($(build_os),linux)
define $(package)_set_vars
  $(package)_config_env=LD_LIBRARY_PATH="$(host_prefix)/lib" PKG_CONFIG_LIBDIR="$(host_prefix)/lib/pkgconfig" CPPFLAGS="-I$(host_prefix)/include" LDFLAGS="-L$(host_prefix)/lib"
endef
endif


define $(package)_config_cmds
  echo '=== config for $(package):' && \
  echo '$($(package)_config_env) $($(package)_conf_tool) $($(package)_config_opts)' && \
  echo '=== ' && \
  $($(package)_config_env) $($(package)_conf_tool) $($(package)_config_opts) 
endef

ifeq ($(build_os),darwin)
define $(package)_build_cmds
  $(MAKE) CPPFLAGS="-I$(host_prefix)/include -fPIC" CFLAGS="-mmacosx-version-min=$(OSX_MIN_VERSION)"
endef
else
define $(package)_build_cmds
  $(MAKE)
endef
endif

define $(package)_stage_cmds
  echo 'Staging dir: $($(package)_staging_dir)$(host_prefix)/' && \
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
