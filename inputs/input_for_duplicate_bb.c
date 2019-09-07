//=============================================================================
// FILE:
//      input_for_duplicate_bb.c
//
// DESCRIPTION:
//      Sample input file for the DuplicateBB pass.
//
// License: MIT
//=============================================================================
long unsigned foo(unsigned arg_1) {
  long unsigned ret_var = 0;
  switch (arg_1) {
  case 1:
    ret_var = 1e1;
    break;
  case 2:
    ret_var = 1e2;
    break;
  case 3:
    ret_var = 1e3;
    break;
  case 4:
    ret_var = 1e4;
    break;
  case 5:
    ret_var = 1e5;
    break;
  case 6:
    ret_var = 1e6;
    break;
  case 7:
    ret_var = 1e7;
    break;
  case 8:
    ret_var = 1e8;
    break;
  case 9:
    ret_var = 1e9;
    break;
  default:
    ret_var = 13;
  }

  return ret_var / 2;
}
