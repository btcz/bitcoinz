package=rust
$(package)_version=1.80.0
$(package)_download_path=https://static.rust-lang.org/dist
$(package)_file_name_linux=rust-$($(package)_version)-x86_64-unknown-linux-gnu.tar.gz
$(package)_sha256_hash_linux=702d3b60816eb9410c84947895c0188e20f588cb91b3aee8d84cec23ddb63582
$(package)_file_name_darwin=rust-$($(package)_version)-x86_64-apple-darwin.tar.gz
$(package)_sha256_hash_darwin=18352bc5802e117189c029b6ed63bf83bfbcde7141f33eee69f037acd1767676
$(package)_file_name_freebsd=rust-$($(package)_version)-x86_64-unknown-freebsd.tar.gz
$(package)_sha256_hash_freebsd=d9eb6a65ee2b51ce982999c093a9f1f2e37f65c3a3d47dff2e0e74b7e01f534c
$(package)_file_name_aarch64_linux=rust-$($(package)_version)-aarch64-unknown-linux-gnu.tar.gz
$(package)_sha256_hash_aarch64_linux=0aa0f0327c8054f3805cc4d83be2c17d1e846d23292aa872ecd6f78d86f84ce5

# Rust-std x86_64-unknown-linux-gnu
$(package)_rust-std-x86_64-pc-linux-gnu_file_name=rust-std-$($(package)_version)-x86_64-unknown-linux-gnu.tar.gz
$(package)_rust-std-x86_64-pc-linux-gnu_sha256_hash=ed301dff3a26da496784ca3de523b0150302fcb001ef71cdcd40ff6d5e2ec75d

# Rust-std x86_64-apple-darwin
$(package)_rust-std-x86_64-apple-darwin_file_name=rust-std-$($(package)_version)-x86_64-apple-darwin.tar.gz
$(package)_rust-std-x86_64-apple-darwin_sha256_hash=069dcd20861c1031a2e1484ef4085503b1e239fdca6b7c6dd4d834c9cc8aff70

# Rust-std aarch64-apple-darwin
$(package)_rust-std-aarch64-apple-darwin_file_name=rust-std-$($(package)_version)-aarch64-apple-darwin.tar.gz
$(package)_rust-std-aarch64-apple-darwin_sha256_hash=ffdbca3f1eaec4fefa6dd461bea44cb2e57a5e1ad0637fc0eabc02ccb8e7e804

# Rust-std aarch64-unknown-linux-gnu
$(package)_rust-std-aarch64-unknown-linux-gnu_file_name=rust-std-$($(package)_version)-aarch64-unknown-linux-gnu.tar.gz
$(package)_rust-std-aarch64-unknown-linux-gnu_sha256_hash=6cdbe8c2b502ca90f42c581b8906b725ccc55bbb3427a332379236bf22be59b3

# Rust-std x86_64-pc-windows-gnu
$(package)_rust-std-x86_64-w64-mingw32_file_name=rust-std-$($(package)_version)-x86_64-pc-windows-gnu.tar.gz
$(package)_rust-std-x86_64-w64-mingw32_sha256_hash=0d6a58268bd94d66280812c042c41521f65acd773e3aae3219eb8a088654be72

$(package)_extra_sources  = $($(package)_rust-std-x86_64-pc-linux-gnu_file_name)
$(package)_extra_sources += $($(package)_rust-std-x86_64-apple-darwin_file_name)
$(package)_extra_sources += $($(package)_rust-std-aarch64-apple-darwin_file_name)
$(package)_extra_sources += $($(package)_rust-std-aarch64-unknown-linux-gnu_file_name)
$(package)_extra_sources += $($(package)_rust-std-x86_64-w64-mingw32_file_name)

ifneq ($(WITH_GUIX),)
# Mapping from GCC canonical hosts to Rust targets
$(package)_rust_target_x86_64-pc-linux-gnu=x86_64-unknown-linux-gnu
$(package)_rust_target_aarch64-unknown-linux-gnu=aarch64-unknown-linux-gnu
$(package)_rust_target_x86_64-apple-darwin=x86_64-apple-darwin
$(package)_rust_target_aarch64-apple-darwin=aarch64-apple-darwin
$(package)_rust_target_x86_64-w64-mingw32=x86_64-pc-windows-gnu

define rust_target
$(package)_rust_target=$($(package)_rust_target_$(canonical_host))
endef

else

# Mapping from GCC canonical hosts to Rust targets
# If a mapping is not present, we assume they are identical, unless $host_os is
# "darwin", in which case we assume x86_64-apple-darwin.
$(package)_rust_target_x86_64-w64-mingw32=x86_64-pc-windows-gnu

define rust_target
$(if $($(1)_rust_target_$(2)),$($(1)_rust_target_$(2)),$(if $(findstring darwin,$(3)),x86_64-apple-darwin,$(2)))
endef

ifneq ($(canonical_host),$(build))
$(package)_rust_target=$(call rust_target,$(package),$(canonical_host),$(host_os))
else
$(package)_rust_target=$(if $(rust_rust_target_$(canonical_host)),$(rust_rust_target_$(canonical_host)),$(canonical_host))
endif
endif

define rust_target
$(if $($(1)_rust_target_$(2)),$($(1)_rust_target_$(2)),$(if $(findstring darwin,$(3)),x86_64-apple-darwin,$(2)))
endef

define $(package)_set_vars
$(package)_stage_opts=--disable-ldconfig
$(package)_stage_build_opts=--without=rust-docs-json-preview,rust-docs
endef

define $(package)_fetch_cmds
echo "DEBUG: canonical_host=$(canonical_host) host_os=$(host_os) build=$(build) build_os=$(build_os) CC=$(CC) CXX=$(CXX)" && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_file_name_$(build_os)),$($(package)_file_name_$(build_os)),$($(package)_sha256_hash_$(build_os))) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_rust-std-x86_64-pc-linux-gnu_file_name),$($(package)_rust-std-x86_64-pc-linux-gnu_file_name),$($(package)_rust-std-x86_64-pc-linux-gnu_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_rust-std-aarch64-apple-darwin_file_name),$($(package)_rust-std-aarch64-apple-darwin_file_name),$($(package)_rust-std-aarch64-apple-darwin_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_rust-std-x86_64-apple-darwin_file_name),$($(package)_rust-std-x86_64-apple-darwin_file_name),$($(package)_rust-std-x86_64-apple-darwin_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_rust-std-aarch64-unknown-linux-gnu_file_name),$($(package)_rust-std-aarch64-unknown-linux-gnu_file_name),$($(package)_rust-std-aarch64-unknown-linux-gnu_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_rust-std-x86_64-w64-mingw32_file_name),$($(package)_rust-std-x86_64-w64-mingw32_file_name),$($(package)_rust-std-x86_64-w64-mingw32_sha256_hash))
endef

define $(package)_extract_cmds
  echo "DEBUG: canonical_host=$(canonical_host) host_os=$(host_os) build=$(build) build_os=$(build_os) CC=$(CC) CXX=$(CXX)" && \
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source_dir)/$($(package)_file_name_$(build_os))" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_rust-std-$(canonical_host)_sha256_hash)  $($(package)_source_dir)/$($(package)_rust-std-$(canonical_host)_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir rust && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_file_name_$(build_os)) -C rust && \
  mkdir rust-std && \
  $(build_TAR) --no-same-owner --strip-components=1 -xf $($(package)_source_dir)/$($(package)_rust-std-$(canonical_host)_file_name) -C rust-std
endef

define $(package)_stage_cmds
  echo "DEBUG: canonical_host=$(canonical_host) host_os=$(host_os) build=$(build) build_os=$(build_os) CC=$(CC) CXX=$(CXX)" && \
  bash ./rust/install.sh --destdir=$($(package)_staging_dir) --prefix=$(build_prefix) $($(package)_stage_opts) $($(package)_stage_build_opts) && \
  bash ./rust-std/install.sh --destdir=$($(package)_staging_dir) --prefix=$(build_prefix) $($(package)_stage_opts)
endef

ifneq ($(WITH_GUIX),)
define $(package)_postprocess_cmds
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/cargo" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/cargo-clippy" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/cargo-fmt" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/clippy-driver" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/rls" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/rust-analyzer" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/rust-demangler" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/rustc" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/rustdoc" && \
  $(BASEDIR)/fix-elf-interpreter.sh "$($(package)_staging_dir)$(host_prefix)/native/bin/rustfmt"
endef
endif
