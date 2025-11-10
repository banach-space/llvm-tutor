import sys

config.llvm_tools_dir = "/usr/local/opt/llvm/bin"
config.llvm_shlib_ext = ".dylib"
config.llvm_shlib_dir = "/Users/naomiarmendariz/llvm-tutor/build/lib"

import lit.llvm
# lit_config is a global instance of LitConfig
lit.llvm.initialize(lit_config, config)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join("/Users/naomiarmendariz/llvm-tutor/build/test")

# Let the main config do the real work.
lit_config.load_config(config, "/Users/naomiarmendariz/llvm-tutor/test/lit.cfg.py")
