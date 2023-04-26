/*! \file
    \brief Утилита umba-sort-headers - сортировка подключенных заголовков в cpp/h файлах
 */

#include "umba/umba.h"
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"

#include "umba/debug_helpers.h"

#include <iostream>
#include <iomanip>
#include <string>
// #include <cstdio>
#include <filesystem>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"

#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/sort_includes.h"

// #include "umba/time_service.h"
// #include "umba/text_utils.h"



umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter logMsg(&coutWriter);
umba::SimpleFormatter logErr(&cerrWriter);
umba::SimpleFormatter logNul(&nulWriter);

bool logWarnType   = true;
bool logGccFormat  = false;
bool logSourceInfo = false;
bool bOverwrite    = false;

marty_cpp::SortIncludeOptions sortIncludeOptions;

std::string inputFilename;
std::string outputFilename;

// bool useClipboard


#include "log.h"

umba::program_location::ProgramLocation<std::string>   programLocationInfo;


#include "umba/cmd_line.h"


#include "app_ver_config.h"
#include "print_ver.h"

#include "arg_parser.h"



int main(int argc, char* argv[])
{

    using namespace umba::omanip;


    auto argsParser = umba::command_line::makeArgsParser( ArgParser()
                                                        , CommandLineOptionCollector()
                                                        , argc, argv
                                                        , umba::program_location::getProgramLocation
                                                            ( argc, argv
                                                            , false // useUserFolder = false
                                                            //, "" // overrideExeName
                                                            )
                                                        );

    // Force set CLI arguments while running under debugger
    if (umba::isDebuggerPresent())
    {
        //argsParser.args.clear();
        //argsParser.args.push_back("@..\\tests\\data\\test01.rsp");

        //argsParser.args.push_back("@..\\make_sources_brief.rsp");
        // argsParser.args.push_back(umba::string_plus::make_string(""));
        // argsParser.args.push_back(umba::string_plus::make_string(""));
        // argsParser.args.push_back(umba::string_plus::make_string(""));
    }

    programLocationInfo = argsParser.programLocationInfo;

    // Job completed - may be, --where option found
    if (argsParser.mustExit)
        return 0;

    if (!argsParser.quet)
    {
        printNameVersion();
    }

    if (!argsParser.parseStdBuiltins())
        return 1;
    if (argsParser.mustExit)
        return 0;

    if (!argsParser.parse())
        return 1;
    if (argsParser.mustExit)
        return 0;


    enum class IoFileType
    {
        nameEmpty,
        regularFile,
        stdioFile, // minus/'-'
        stdinFile,
        stdoutFile,
        clipboard
    };

    auto detectFilenameType = [](const std::string &n) -> IoFileType
    {
        if (n.empty())
            return IoFileType::nameEmpty;

        std::string N = marty_cpp::toUpper(n);

        if (N=="-")
            return IoFileType::stdioFile;

        if (N=="{STDIN}")
            return IoFileType::stdinFile;

        if (N=="{STDOUT}")
            return IoFileType::stdoutFile;

        if (N=="{CLPB}" || N=="{CLPBRD}" || N=="{CLIPBRD}" || N=="{CLIPBOARD}")
            return IoFileType::clipboard;

        return IoFileType::regularFile;
    };


    IoFileType inputFileType = detectFilenameType(inputFilename);

    if (inputFileType==IoFileType::nameEmpty)
    {
        LOG_ERR_OPT << "no input file name taken\n";
        return 1;
    }

    if (inputFileType==IoFileType::stdoutFile)
    {
        LOG_ERR_OPT << "can't use STDOUT for input\n";
        return 1;
    }

    if (inputFileType==IoFileType::stdioFile)
    {
        inputFileType = IoFileType::stdinFile; // заменяем минус на STDIN для входного файла
    }

    //------------------------------
    
    if (outputFilename.empty())
    {
        if (inputFileType==IoFileType::stdinFile)
        {
            outputFilename = "-";
        }
        else
        {
            outputFilename = inputFilename;
        }
    }

    //------------------------------
    
    IoFileType outputFileType = detectFilenameType(outputFilename);

    if (outputFileType==IoFileType::nameEmpty)
    {
        LOG_ERR_OPT << "no output file name taken\n";
        return 1;
    }

    if (outputFileType==IoFileType::stdinFile)
    {
        LOG_ERR_OPT << "can't use STDIN for output\n";
        return 1;
    }

    if (outputFileType==IoFileType::stdioFile)
    {
        outputFileType = IoFileType::stdoutFile; // заменяем минус на STDOUT для выходного файла
    }




// std::string inputFilename;
// std::string outputFilename;




        // LOG_WARN_OPT("input-not-exist")<<"failed to open input file '"<<fp.first<<"'\n";
        // LOG_MSG_OPT<<"opening output file" << endl;

    // appConfig = appConfig.getAdjustedConfig(programLocationInfo);
    // //pAppConfig = &appConfig;
    //  
    // if (appConfig.getOptShowConfig())
    // {
    //     printInfoLogSectionHeader(logMsg, "Actual Config");
    //     // logMsg << appConfig;
    //     appConfig.print(logMsg) << "\n";
    // }
    //  
    // if (appConfig.outputName.empty())
    // {
    //     LOG_ERR_OPT << "output name not taken" << endl;
    //     return 1;
    // }
    
    return 0;
}