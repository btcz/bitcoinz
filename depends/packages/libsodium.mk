package=libsodium
$(package)_version=1.0.19
$(package)_download_path=https://download.libsodium.org/libsodium/releases/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=018d79fe0a045cca07331d37bd0cb57b2e838c51bc48fd837a1472e50068bbea
$(package)_dependencies=
$(package)_config_opts=
# clang won't inline the ARM crypto intrinsics of 1.0.19's aes256gcm code
# without the feature enabled. Every Apple Silicon CPU has it.
$(package)_cflags_aarch64_darwin=-march=armv8-a+crypto

define $(package)_preprocess_cmds
  cd $($(package)_build_subdir); DO_NOT_UPDATE_CONFIG_SCRIPTS=1 ./autogen.sh
endef

define $(package)_config_cmds
  $($(package)_autoconf) --enable-static --disable-shared
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
