package=rust
$(package)_version=1.69.0
$(package)_download_path=https://static.rust-lang.org/dist

$(package)_file_name_linux=rust-$($(package)_version)-x86_64-unknown-linux-gnu.tar.gz
$(package)_sha256_hash_linux=2ca4a306047c0b8b4029c382910fcbc895badc29680e0332c9df990fd1c70d4f
$(package)_file_name_darwin=rust-$($(package)_version)-x86_64-apple-darwin.tar.gz
$(package)_sha256_hash_darwin=9818dab2c3726d63dfbfde12c9273e62e484ef6d6f6e05a6431a3e089c335454
$(package)_file_name_mingw32=rust-$($(package)_version)-x86_64-pc-windows-gnu.tar.gz
$(package)_sha256_hash_mingw32=9a2a887defb9d0e0aa48fd54237b583a693a2565420b4d90efd0740c0fe69b0d

define $(package)_set_vars
$(package)_stage_opts=--disable-ldconfig
$(package)_stage_build_opts=--without=rust-docs-json-preview,rust-docs
endef

ifeq ($(build_os),darwin)
$(package)_file_name=$($(package)_file_name_darwin)
$(package)_sha256_hash=$($(package)_sha256_hash_darwin)
else ifeq ($(host_os),mingw32)
$(package)_file_name=$($(package)_file_name_mingw32)
$(package)_sha256_hash=$($(package)_sha256_hash_mingw32)
else
$(package)_file_name=$($(package)_file_name_linux)
$(package)_sha256_hash=$($(package)_sha256_hash_linux)
endif

ifeq ($(host_os),mingw32)
$(package)_build_subdir=buildos
$(package)_extra_sources = $($(package)_file_name_$(build_os))

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_file_name_$(build_os)),$($(package)_file_name_$(build_os)),$($(package)_sha256_hash_$(build_os)))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_sha256_hash_$(build_os))  $($(package)_source_dir)/$($(package)_file_name_$(build_os))" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir mingw32 && \
  tar --strip-components=1 -xf $($(package)_source) -C mingw32 && \
  mkdir buildos && \
  tar --strip-components=1 -xf $($(package)_source_dir)/$($(package)_file_name_$(build_os)) -C buildos
endef

define $(package)_stage_cmds
  ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native $($(package)_stage_opts) $($(package)_stage_build_opts) && \
  cp -r ../mingw32/rust-std-x86_64-pc-windows-gnu/lib/rustlib/x86_64-pc-windows-gnu $($(package)_staging_dir)$(host_prefix)/native/lib/rustlib
endef
else

define $(package)_stage_cmds
  ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native $($(package)_stage_opts) $($(package)_stage_build_opts)
endef
endif
