package=libgmp
$(package)_version=6-1-2
$(package)_download_path=https://github.com/ezzygarmyz/libgmp/releases/download/6.1.2/
$(package)_file_name=libgmp-$($(package)_version).tar.bz2
$(package)_sha256_hash=77613c44076fd04b3b14a94e77da8bf8153443cca0f0caa517bc6c46479ea77e
$(package)_dependencies=
$(package)_config_opts=--enable-cxx --disable-shared --with-pic

define $(package)_config_cmds
  $($(package)_autoconf) --host=$(host) --build=$(build)
endef

define $(package)_build_cmds
  $(MAKE) CPPFLAGS='-fPIC'
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install ; echo '=== staging find for $(package):' ; find $($(package)_staging_dir)
endef
