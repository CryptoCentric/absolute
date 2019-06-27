package=native_ccache
$(package)_version=3.7.1
$(package)_download_path=https://github.com/ccache/ccache/releases/download/v$($(package)_version)
$(package)_file_name=ccache-$($(package)_version).tar.bz2
$(package)_sha256_hash=1c6501d5bd01952f5535c3f11ca774aedf4711e373a7bea7e0b1f487f0789b19
https://github.com/ccache/ccache/releases/download/v3.7.1/ccache-3.7.1.tar.bz2
define $(package)_set_vars
$(package)_config_opts=
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm -rf lib include
endef
