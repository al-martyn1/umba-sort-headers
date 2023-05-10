#pragma once

#include <string>
#include <vector>
#include <map>

#include "umba/program_location.h"
#include "umba/enum_helpers.h"
#include "umba/flag_helpers.h"

#include "umba/regex_helpers.h"

//----------------------------------------------------------------------------



struct FilenamePair
{
    std::string inputFilename ;
    std::string outputFilename;
};

//----------------------------------------------------------------------------
struct AppConfig
{

    bool                                     scanMode = false;
    std::vector<std::string>                 scanPaths;
    std::vector<std::string>                 includeFilesMaskList;
    std::vector<std::string>                 excludeFilesMaskList;

}; // struct AppConfig




