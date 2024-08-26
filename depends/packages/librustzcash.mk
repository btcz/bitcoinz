package=librustzcash
$(package)_version=0.1
$(package)_download_path=https://github.com/zcash/$(package)/archive/
$(package)_file_name=$(package)-$($(package)_git_commit).tar.gz
$(package)_download_file=$($(package)_git_commit).tar.gz
$(package)_sha256_hash=9909ec59fa7a411c2071d6237b3363a0bc6e5e42358505cf64b7da0f58a7ff5a
$(package)_git_commit=06da3b9ac8f278e5d4ae13088cf0a4c03d2c13f5
$(package)_dependencies=rust
$(package)_dependencies+=$(rust_crates)
$(package)_patches=cargo.config guix.config 0001-Start-using-cargo-clippy-for-CI.patch remove-dev-dependencies.diff

# Mapping from GCC canonical hosts to Rust targets
$(package)_rust_target_x86_64-pc-linux-gnu=x86_64-unknown-linux-gnu
$(package)_rust_target_aarch64-unknown-linux-gnu=aarch64-unknown-linux-gnu
$(package)_rust_target_x86_64-apple-darwin=x86_64-apple-darwin
$(package)_rust_target_x86_64-w64-mingw32=x86_64-pc-windows-gnu

ifneq ($(WITH_GUIX),)
$(package)_rust_target=$($(package)_rust_target_$(canonical_host))
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

ifneq ($(WITH_GUIX),)
$(package)_library_file=target/$($(package)_rust_target)/release/librustzcash.a
else ifneq ($(canonical_host),$(build))
$(package)_library_file=target/$($(package)_rust_target)/release/librustzcash.a
else
$(package)_library_file=target/release/librustzcash.a
endif

define $(package)_set_vars
$(package)_build_opts=--release
$(package)_build_opts+=--frozen
ifneq ($(WITH_GUIX),)
$(package)_build_opts+=--target=$($(package)_rust_target)
else ifneq ($(canonical_host),$(build))
$(package)_build_opts+=--target=$($(package)_rust_target)
endif
endef

ifneq ($(WITH_GUIX),)
define $(package)_config_cmds
  cat $($(package)_patch_dir)/guix.config >> .cargo/config
endef
endif

define $(package)_preprocess_cmds
  patch -p1 -d pairing < $($(package)_patch_dir)/0001-Start-using-cargo-clippy-for-CI.patch && \
  patch -p1 < $($(package)_patch_dir)/remove-dev-dependencies.diff && \
  mkdir .cargo && \
  cat $($(package)_patch_dir)/cargo.config | sed 's|CRATE_REGISTRY|$(host_prefix)/$(CRATE_REGISTRY)|' > .cargo/config
endef

define $(package)_build_cmds
  echo "DEBUG: canonical_host=$(canonical_host) host_os=$(host_os) build=$(build) build_os=$(build_os) CC=$(CC) CXX=$(CXX)" && \
  echo "$(host_prefix)/native/bin/cargo build --package librustzcash $($(package)_build_opts)" && \
  $(host_prefix)/native/bin/cargo build --package librustzcash $($(package)_build_opts)
endef

define $(package)_stage_cmds
  mkdir $($(package)_staging_dir)$(host_prefix)/lib/ && \
  mkdir $($(package)_staging_dir)$(host_prefix)/include/ && \
  cp $($(package)_library_file) $($(package)_staging_dir)$(host_prefix)/lib/ && \
  cp librustzcash/include/librustzcash.h $($(package)_staging_dir)$(host_prefix)/include/
endef
