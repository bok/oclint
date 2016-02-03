#include <llvm/Config/llvm-config.h>
#include <llvm/Support/Path.h>

#include "oclint/helper/ExcludeHelper.h"

static llvm::SmallString<1024> remove_dots(llvm::StringRef path, bool remove_dot_dot) {
    using namespace llvm;
    using namespace llvm::sys;

    SmallVector<StringRef, 16> components;

    // Skip the root path, then look for traversal in the components.
    StringRef rel = path::relative_path(path);
    for (StringRef C : llvm::make_range(path::begin(rel), path::end(rel))) {
        if (C == ".")
            continue;
        if (C == "..") {
            if (!components.empty())
                components.pop_back();
            continue;
        }
        components.push_back(C);
    }

   SmallString<1024> buffer(path::root_path(path).str());
   for (StringRef C : components)
        path::append(buffer, C);
   return buffer;
}

static bool remove_dots(llvm::SmallVectorImpl<char> &path, bool remove_dot_dot) {
    llvm::StringRef p(path.data(), path.size());

    llvm::SmallString<1024> result = remove_dots(p, remove_dot_dot);
    if (result == path)
        return false;

    path.swap(result);
    return true;
}

llvm::SmallString<1024> getAbsolutePath(llvm::StringRef path) {
    using namespace llvm;
    using namespace llvm::sys;

    SmallString<1024> absolutePath(path.str());
    fs::make_absolute(absolutePath);
#if (LLVM_VERSION_MAJOR > 3) || ((LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR > 7))
    path::remove_dots(absolutePath, true);
#else
    remove_dots(absolutePath, true);
#endif
    return absolutePath;
}


bool locationIsExcluded(clang::SourceLocation sourceLocation, std::vector<std::string> *excludePaths, clang::SourceManager *sourceManager) {
    if (!excludePaths || !sourceLocation.isValid()) {
        return false;
    }

    // all system headers and macros are excluded
    if (sourceManager->isInSystemMacro(sourceLocation)) {
        return true;
    }

    // get the filename
    clang::StringRef fileName = sourceManager->getFilename(sourceLocation);
    if (fileName.empty() && sourceLocation.isMacroID()) {
        // for macros, we want the place of definition
        fileName = sourceManager->getFilename(sourceManager->getSpellingLoc(sourceLocation));
    }

    std::string filePath = getAbsolutePath(fileName).str();
    llvm::StringRef file(filePath);

    for (const auto& excludePath : *excludePaths)
    {
        std::string exclude = getAbsolutePath(excludePath).str();

        if (file.startswith(exclude)) {
            return true;
        }
    }
    return false;
}
