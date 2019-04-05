# -*- Python -*-

# Configuration file for the 'lit' test runner.

import lit.util
import lit.formats
from lit.llvm import llvm_config
from lit.llvm.subst import FindTool
from lit.llvm.subst import ToolSubst

# name: The name of this test suite.
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
config.suffixes = ['.ll']

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

llvm_config.use_default_substitutions()

tools = ["opt"]
llvm_config.add_tool_substitutions(tools, config.llvm_tools_dir)
