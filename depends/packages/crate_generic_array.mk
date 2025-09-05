package=crate_generic_array
$(package)_crate_name=generic-array
$(package)_version=0.9.1
$(package)_download_path=https://static.crates.io/crates/$($(package)_crate_name)
$(package)_file_name=$($(package)_crate_name)-$($(package)_version).crate
$(package)_sha256_hash=6d00328cedcac5e81c683e5620ca6a30756fc23027ebf9bff405c0e8da1fbb7e
$(package)_crate_versioned_name=$($(package)_crate_name)

define $(package)_preprocess_cmds
  $(call generate_crate_checksum,$(package))
endef

define $(package)_stage_cmds
  $(call vendor_crate_source,$(package))
endef
