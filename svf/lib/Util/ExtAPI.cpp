//===- ExtAPI.cpp -- External functions -----------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * ExtAPI.cpp
 *
 *  Created on: July 1, 2022
 *      Author: Shuangxiang Kan
 */

#include "Util/ExtAPI.h"
#include "Util/SVFUtil.h"
#include "Util/Options.h"
#include "Util/config.h"
#include <ostream>
#include <sys/stat.h>

using namespace SVF;

ExtAPI* ExtAPI::extOp = nullptr;
std::string ExtAPI::extBcPath = "";

ExtAPI* ExtAPI::getExtAPI()
{
    if (extOp == nullptr)
    {
        extOp = new ExtAPI;
    }
    return extOp;
}

void ExtAPI::destory()
{
    if (extOp != nullptr)
    {
        delete extOp;
        extOp = nullptr;
    }
}

// Set extapi.bc file path
bool ExtAPI::setExtBcPath(const std::string& path)
{
    struct stat statbuf;
    if (!path.empty() && !stat(path.c_str(), &statbuf))
    {
        extBcPath = path;
        return true;
    }
    return false;
}

// Get environment variables $SVF_DIR and "npm root" through popen() method
static std::string GetStdoutFromCommand(const std::string& command)
{
    char buffer[128];
    std::string result = "";
    // Open pipe to file
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        return "popen failed!";
    }
    // read till end of process:
    while (!feof(pipe))
    {
        // use buffer to read and add to result
        if (fgets(buffer, 128, pipe) != NULL) result += buffer;
    }
    pclose(pipe);
    // remove "\n"
    result.erase(remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

// Get extapi.bc file path in npm
static std::string getFilePath(const std::string& path)
{
    std::string bcFilePath = "";
    if (path.compare("SVF_DIR") == 0)
    {
        char const* svfdir = getenv("SVF_DIR");
        if (svfdir) bcFilePath = svfdir;
    }
    else if (path.compare("npm root") == 0)
    {
        bcFilePath = GetStdoutFromCommand(path);
        bcFilePath.append("/SVF");
    }

    if (!bcFilePath.empty() && bcFilePath.back() != '/') bcFilePath.push_back('/');
    bcFilePath.append(SVF_BUILD_TYPE "-build").append("/lib/extapi.bc");
    return bcFilePath;
}

// Get extapi.bc path
std::string ExtAPI::getExtBcPath()
{
    // Default ways of retrieving extapi.bc (in order of precedence):
    //  1.  Set `path/to/extapi.bc` through `setExtBcPath()`
    //  2.  Set `path/to/extapi.bc` through the command line argument `-extapi=path/to/extapi.bc`
    //  3.  Get location generated by CMakeLists.txt from `config.h` header file (if SVF was installed)
    //  4.  Get location in build tree based from `config.h` header file (if SVF was only built)
    //  5.  Get location based on environment variable $ENV{SVF_DIR}
    //  6.  Search for `extapi.bc` from root directory for npm installation (iff SVF installed through npm)

    //  1.  Set `path/to/extapi.bc` through `setExtBcPath()`
    if (!extBcPath.empty()) return extBcPath;

    //  2.  Set `path/to/extapi.bc` through the command line argument `-extapi=path/to/extapi.bc`
    if (setExtBcPath(Options::ExtAPIPath())) return extBcPath;

    //  3.  Get location generated by CMakeLists.txt from `config.h` header file (if SVF was installed)
    if (setExtBcPath(SVF_EXTAPI_BC)) // Full path is available (for custom file names)
        return extBcPath;
    if (setExtBcPath(SVF_EXTAPI_DIR "/extapi.bc")) // Based on directory & default filename
        return extBcPath;

    //  4.  Get location in build tree based from `config.h` header file (if SVF was only built)
    if (setExtBcPath(SVF_BUILD_DIR "/lib/extapi.bc")) return extBcPath;

    //  5.  Get location based on environment variable $ENV{SVF_DIR}
    if (setExtBcPath(getFilePath("SVF_DIR"))) return extBcPath;

    //  6.  Search for `extapi.bc` from root directory for npm installation (iff SVF installed through npm)
    if (setExtBcPath(getFilePath("npm root"))) return extBcPath;

    SVFUtil::errs() << "ERROR: Failed to find \"extapi.bc\" LLVM bitcode file in " << extBcPath << std::endl
                    << "To override the default locations for \"extapi.bc\", you can:" << std::endl
                    << "\t1. Use the command line argument \"-extapi=path/to/extapi.bc\"" << std::endl
                    << "\t2. Use the \"setExtBcPath()\" function *BEFORE* calling \"buildSVFModule()\"" << std::endl
                    << "\t3. Override the paths in \"svf/Util/config.h\" (WARNING: will be overwritten when "
                    << "rebuilding SVF (generated by CMakeLists.txt))" << std::endl;
    abort();
}

std::string ExtAPI::getExtFuncAnnotation(const SVFFunction* fun, const std::string& funcAnnotation)
{
    assert(fun && "Null SVFFunction* pointer");
    for (const std::string& annotation : fun->getAnnotations())
        if (annotation.find(funcAnnotation) != std::string::npos) return annotation;
    return "";
}

bool ExtAPI::hasExtFuncAnnotation(const SVFFunction* fun, const std::string& funcAnnotation)
{
    assert(fun && "Null SVFFunction* pointer");
    for (const std::string& annotation : fun->getAnnotations())
        if (annotation.find(funcAnnotation) != std::string::npos) return true;
    return false;
}

bool ExtAPI::is_memcpy(const SVFFunction* F)
{
    return F && (hasExtFuncAnnotation(F, "MEMCPY") || hasExtFuncAnnotation(F, "STRCPY") ||
                 hasExtFuncAnnotation(F, "STRCAT"));
}

bool ExtAPI::is_memset(const SVFFunction* F)
{
    return F && hasExtFuncAnnotation(F, "MEMSET");
}

bool ExtAPI::is_alloc(const SVFFunction* F)
{
    return F && hasExtFuncAnnotation(F, "ALLOC_RET");
}

// Does (F) allocate a new object and assign it to one of its arguments?
bool ExtAPI::is_arg_alloc(const SVFFunction* F)
{
    return F && hasExtFuncAnnotation(F, "ALLOC_ARG");
}

// Get the position of argument which holds the new object
s32_t ExtAPI::get_alloc_arg_pos(const SVFFunction* F)
{
    std::string allocArg = getExtFuncAnnotation(F, "ALLOC_ARG");
    assert(!allocArg.empty() && "Not an alloc call via argument or incorrect extern function annotation!");

    std::string number;
    for (char c : allocArg)
    {
        if (isdigit(c)) number.push_back(c);
    }
    assert(!number.empty() && "Incorrect naming convention for svf external functions(ALLOC_ARG + number)?");
    return std::stoi(number);
}

// Does (F) reallocate a new object?
bool ExtAPI::is_realloc(const SVFFunction* F)
{
    return F && hasExtFuncAnnotation(F, "REALLOC_RET");
}

// Should (F) be considered "external" (either not defined in the program
//   or a user-defined version of a known alloc or no-op)?
bool ExtAPI::is_ext(const SVFFunction* F)
{
    assert(F && "Null SVFFunction* pointer");
    if (F->isDeclaration() || F->isIntrinsic()) return true;
    else if (hasExtFuncAnnotation(F, "OVERWRITE") && F->getAnnotations().size() == 1)
        return false;
    else
        return !F->getAnnotations().empty();
}
