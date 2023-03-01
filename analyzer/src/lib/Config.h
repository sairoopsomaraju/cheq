#ifndef SACONFIG_H
#define SACONFIG_H

#include "llvm/Support/FileSystem.h"

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <fstream>

//
// Configurations for compilation.
//
//#define SOUND_MODE 1

// Skip functions with more blocks to avoid scalability issues
#define MAX_BLOCKS_SUPPORT 500

//
// Function modeling
//

// Setup functions that handle errors
static void SetErrorHandleFuncs(std::set<std::string> &ErrorHandleFuncs) {

	std::string exepath = llvm::sys::fs::getMainExecutable(NULL, NULL);
	std::string exedir = exepath.substr(0, exepath.find_last_of('/'));
	std::string line;
  std::ifstream errfile(exedir	+ "/configs/err-funcs");
  if (errfile.is_open()) {
		while (!errfile.eof()) {
			getline (errfile, line);
			if (line.length() > 1) {
				ErrorHandleFuncs.insert(line);
			}
		}
    errfile.close();
  }

	std::string ErrorHandleFN[] = {
		"BUG",
		"BUG_ON",
		"ASM_BUG",
		"panic",
		"ASSERT",
		"assert",
		"dump_stack",
		"__warn_printk",
		"usercopy_warn",
		"signal_fault",
		"pr_err",
		"pr_warn",
		"pr_warning",
		"pr_alert",
		"pr_emerg",
		"pr_crit",
	};
	for (auto F : ErrorHandleFN) {
		ErrorHandleFuncs.insert(F);
	}
}

// Setup functions that copy/move/cast values.
static void SetCopyFuncs(
		// <src, dst, size>
		std::map<std::string, std::tuple<int8_t, int8_t, int8_t>> &CopyFuncs) {

	CopyFuncs["memcpy"] = std::make_tuple(1, 0, 2);
	CopyFuncs["__memcpy"] = std::make_tuple(1, 0, 2);
	CopyFuncs["llvm.memcpy.p0i8.p0i8.i32"] = std::make_tuple(1, 0, 2);
	CopyFuncs["llvm.memcpy.p0i8.p0i8.i64"] = std::make_tuple(1, 0, 2);
	CopyFuncs["strncpy"] = std::make_tuple(1, 0, 2);
	CopyFuncs["memmove"] = std::make_tuple(1, 0, 2);
	CopyFuncs["__memmove"] = std::make_tuple(1, 0, 2);
	CopyFuncs["llvm.memmove.p0i8.p0i8.i32"] = std::make_tuple(1, 0, 2);
	CopyFuncs["llvm.memmove.p0i8.p0i8.i64"] = std::make_tuple(1, 0, 2);
}

#endif
