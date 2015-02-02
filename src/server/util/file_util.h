////////////////////////////////////////////////////////////////////////////
//    File:        file_util.h
//    Author:      Ken Chatfield
//    Description: Misc helpers for working with files
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_UTILS_FILE_UTIL_H_
#define CPUVISOR_UTILS_FILE_UTIL_H_

#include <string>

namespace cpuvisor {

  bool relativePath(const std::string& full_path, const std::string& base_path,
                    std::string* rel_path);

}

#endif
