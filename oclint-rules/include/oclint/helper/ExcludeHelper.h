#ifndef OCLINT_HELPER_EXCLUDEHELPER_H
#define OCLINT_HELPER_EXCLUDEHELPER_H

#include <string>
#include <vector>

#include <clang/Basic/SourceManager.h>

llvm::SmallString<1024> getAbsolutePath(llvm::StringRef path);
bool isExcluded(clang::SourceLocation sourceLocation, std::vector<std::string> *excludePaths, clang::SourceManager *sourceManager);

#endif
