#! /bin/env bash
# === static_registration.sh ==================================================
#  Copy the MBASub pass into llvm-project/llvm and register it statically
# 
#  DESCRIPTION:
#   This script copies MBASub into llvm-project/llvm and makes all the required
#   changes to register it as a static opt plugin.
#
#   After running this script you will have to (re-)build opt. The following
#   additional CMake options have to be specified:
#     -DLLVM_BUILD_EXAMPLES=On -DLLVM_MBASUB_LINK_INTO_TOOLS=ON
#
#   Once opt is (re-)build, you can run MBASub like this:
#     $LLVM_BUILD_DIR/bin/opt --passes=mba-sub -S $LLVM_TUTOR_DIR/test/MBA_sub.ll
#   Thanks to static regisration, you don't have to load the plugin with e.g.
#   `-load`.
#
#  USAGE:
#    export $LLVM_DIR=<llvm-project/source/dir>
#    cd <llvm-tutor/source/dir>
#    bash utils/static_registration.sh --llvm_project_dir $LLVM_DIR 
#
# =============================================================================
set -euo pipefail

# The location of llvm-tutor. The default value will normally work perfectly
# fine.
LLVM_TUTOR_DIR="$(dirname "$0")/../"
# The location of llvm-project
LLVM_PROJECT_DIR=""
# The location of MBASub within llvm-project (i.e. where MBSub is copied to)
LLVM_PASS_DIR=""

usage()
{
    echo "usage: static_registration [[[-l llvm_project_dir]] | [-h]]"
}

parse_args()
{
  while [ "${1:-}" != "" ]; do
      case $1 in
          -l | --llvm_project_dir )   shift 
                                      LLVM_PROJECT_DIR=$1
                                      ;;
          -h | --help )               usage
                                      exit
                                      ;;
          * )                         usage
                                      exit 1
      esac
      shift
  done
}

# === copy_mbasub_into_llvm ====================================================
#
# Copies the source and test files for MBASub into llvm-project/llvm
# ==============================================================================
copy_mbasub_into_llvm()
{
  # The directory within llvm for tests for MBASub
  readonly llvm_pass_test_dir="$LLVM_PROJECT_DIR/llvm/test/Examples/MBASub"

  # Clean-up
  rm -rf "$LLVM_PASS_DIR"
  rm -rf "$llvm_pass_test_dir"

  # Create dirs within llvm-project
  mkdir "$LLVM_PASS_DIR"
  mkdir "$llvm_pass_test_dir"

  # Copy the source and test files accross to llvm-project/llvm
  cp "$LLVM_TUTOR_DIR/lib/MBASub.cpp" "$LLVM_PASS_DIR"
  cp "$LLVM_TUTOR_DIR/include/MBASub.h" "$LLVM_PASS_DIR"
  cp "$LLVM_TUTOR_DIR/test/MBA_sub.ll" "$llvm_pass_test_dir"
}

# === add_cmake_for_mbasub =====================================================
#
# Creates a CMake file for MBASub
#
# This a brand new CMake script that is added in llvm-project/llvm.
# ==============================================================================
add_cmake_for_mbasub()
{
  touch "$LLVM_PASS_DIR/CMakeLists.txt"

  # NOTE: the following heredoc uses tabs rather than spaces for indentation.
  # These tabs will be suppressed when the command (i.e. cat) is executed.
  # That's thanks to the delimiter token, EOF, being prefixed with `-`.
  cat > "$LLVM_PASS_DIR/CMakeLists.txt" <<-'EOF'
	if(LLVM_MBASUB_LINK_INTO_TOOLS)
		message(WARNING "Setting LLVM_MBASUB_LINK_INTO_TOOLS=ON only makes sense for testing purpose")
	endif()

	# The plugin expects to not link against the Support and Core libraries,
	# but expects them to exist in the process loading the plugin. This doesn't
	# work with DLLs on Windows (where a shared library can't have undefined
	# references), so just skip this example on Windows.
	if (NOT WIN32)
		add_llvm_pass_plugin(MBASub
			MBASub.cpp
			DEPENDS
			intrinsics_gen
			BUILDTREE_ONLY
		 )

		install(TARGETS ${name} RUNTIME DESTINATION examples)
		set_target_properties(${name} PROPERTIES FOLDER "Examples")
	endif()
EOF
}

# === add_mbasumb_subdir_in_cmake ===============================================
#
# Adds MBASub subdirectory in the parent CMake script
#
# Once MBASub is copied into llvm-project/llvm, the CMake script from the
# parent directory needs to be aware of this new subdirectory. This function
# adds the required line. Without this change CMake would skip the MBASub
# sub-directory that was created within llvm-project/llvm.
# ==============================================================================
add_mbasub_subdir_in_cmake()
{
  
  cd "$LLVM_PROJECT_DIR"

  # NOTE: the following heredoc uses tabs rather than spaces for indentation.
  # These tabs will be suppressed when the command (i.e. cat) is executed.
  # That's thanks to the delimiter token, EOF, being prefixed with `-`.
  cat > file.diff <<-'EOF'
  diff --git a/llvm/examples/CMakeLists.txt b/llvm/examples/CMakeLists.txt
  index 74613bd1350b..917bb2e59371 100644
  --- a/llvm/examples/CMakeLists.txt
  +++ b/llvm/examples/CMakeLists.txt
  @@ -8,6 +8,7 @@ add_subdirectory(ModuleMaker)
   add_subdirectory(OrcV2Examples)
   add_subdirectory(SpeculativeJIT)
   add_subdirectory(Bye)
  +add_subdirectory(MBASub)

   if(LLVM_ENABLE_EH AND (NOT WIN32) AND (NOT "${LLVM_NATIVE_ARCH}" STREQUAL "ARM"))
       add_subdirectory(ExceptionDemo)
EOF

  patch -p1 < file.diff
}


# === update_mbasub_test ======================================================
#
# Updates the in-tree version of MBA_sub.ll so that it doesn't use `-load`
#
# Once MBA_sub.ll is copied into llvm-project/llvm, it has to be updated not to
# use `-load` (legacy PM) or `-load-pass-plugin` (new PM) when running the
# MBASub plugin. These options are not needed in-tree as this script registers
# MBASub as a static plugin.
# =============================================================================
update_mbasub_test()
{
  cd "$LLVM_PROJECT_DIR"

  # NOTE: the following heredoc uses tabs rather than spaces for indentation.
  # These tabs will be suppressed when the command (i.e. cat) is executed.
  # That's thanks to the delimiter token, EOF, being prefixed with `-`.
  cat > file.diff <<-'EOF'
	 define signext i32 @foo(i32 signext, i32 signext, i32 signext, i32 signext) {
	   %5 = sub i32 %1, %0
	diff --git a/llvm/test/Examples/MBASub/MBA_sub.ll b/llvm/test/Examples/MBASub/MBA_sub.ll
  index c33a8b5..a031512 100644
	--- a/llvm/test/Examples/MBASub/MBA_sub.ll
	+++ b/llvm/test/Examples/MBASub/MBA_sub.ll
  @@ -1,4 +1,4 @@
  -; RUN:  opt -load-pass-plugin=%shlibdir/libMBASub%shlibext -passes="mba-sub" -S %s \
  +; RUN:  opt  -passes="mba-sub" -S %s \
   ; RUN:  | FileCheck %s

   define signext i32 @foo(i32 signext, i32 signext, i32 signext, i32 signext) {
EOF

  patch -p1 < file.diff
}

# === main ====================================================================
#
# Entry point for this script
# =============================================================================
main()
{
  parse_args "$@"
  LLVM_PASS_DIR="$LLVM_PROJECT_DIR/llvm/examples/MBASub"

  copy_mbasub_into_llvm
  add_cmake_for_mbasub
  add_mbasub_subdir_in_cmake
  update_mbasub_test
}

main "$@"
