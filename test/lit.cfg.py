# -*- Python -*-

# Configuration file for the 'lit' test runner.

import platform

import lit.util
import lit.formats
# Global instance of LLVMConfig provided by lit
from lit.llvm import llvm_config
from lit.llvm.subst import FindTool
from lit.llvm.subst import ToolSubst

# name: The name of this test suite.
# (config is an instance of TestingConfig created when discovering tests)
config.name = 'LLVM-TUTOR'

# testFormat: The test format to use to interpret tests.
# As per shtest.py (my formatting):
#   ShTest is a format with one file per test. This is the primary format for
#   regression tests (...)
# I couldn't find any more documentation on this, but it seems to be exactly
# what we want here.
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)

# suffixes: A list of file extensions to treat as test files. This is overriden
# by individual lit.local.cfg files in the test subdirectories.
config.suffixes = ['.ll', '.c']

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

llvm_config.use_default_substitutions()

# On Mac OS, 'clang' installed via HomeBrew (or build from sources) won't know
# where to look for standard headers (e.g. 'stdlib.h'). This is a workaround.
if platform.system() == 'Darwin':
    tool_substitutions = [
        ToolSubst('%clang', "clang",
            extra_args=["-isysroot",
                # http://lists.llvm.org/pipermail/cfe-dev/2016-July/049868.html
                "`xcrun --show-sdk-path`"]),
    ]
else:
    tool_substitutions = [
        ToolSubst('%clang', "clang",
            )]
llvm_config.add_tool_substitutions(tool_substitutions)

# The list of tools required for testing - prepend them with the path specified
# during configuration (i.e. LT_LIT_TOOLS_DIR).
tools = ["opt", "not", "FileCheck", "clang"]
llvm_config.add_tool_substitutions(tools, config.llvm_tools_dir)

# The extension for shared libraries depends on the platform - this takes care of
# it.
config.substitutions.append(('%shlibext', config.llvm_shlib_ext))
