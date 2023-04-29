/*! \file
    \brief Утилита umba-sort-headers - сортировка подключенных заголовков в cpp/h файлах
 */

// Должна быть первой
#include "umba/umba.h"
//---

//#-sort
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"
//#+sort

#include "umba/debug_helpers.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"

#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/sort_includes.h"
#include "marty_cpp/enums.h"
#include "marty_cpp/src_normalization.h"

#include "encoding/encoding.h"
#include "umba/cli_tool_helpers.h"


#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


// #include "umba/time_service.h"
// #include "umba/text_utils.h"



umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter umbaLogStreamErr(&coutWriter);
umba::SimpleFormatter umbaLogStreamMsg(&cerrWriter);
umba::SimpleFormatter umbaLogStreamNul(&nulWriter);

bool umbaLogGccFormat   = false; // true;
bool umbaLogSourceInfo  = false;

bool bOverwrite    = false;

marty_cpp::SortIncludeOptions sortIncludeOptions;

std::string inputFilename;
std::string outputFilename;

using marty_cpp::ELinefeedType;
ELinefeedType outputLinefeed = ELinefeedType::detect;

// bool useClipboard


#include "log.h"

umba::program_location::ProgramLocation<std::string>   programLocationInfo;


#include "umba/cmd_line.h"

// Конфиг версии
#include "app_ver_config.h"
// Принтуем версию
#include "print_ver.h"
// Парсер параметров командной строки
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

    // try
    // {
        // Job completed - may be, --where option found
        if (argsParser.mustExit)
            return 0;
       
        // if (!argsParser.quet)
        // {
        //     printNameVersion();
        // }
       
        if (!argsParser.parseStdBuiltins())
            return 1;
        if (argsParser.mustExit)
            return 0;
       
        if (!argsParser.parse())
            return 1;
        if (argsParser.mustExit)
            return 0;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << e.what() << "\n";
    //     return -1;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << "command line arguments parsing error" << "\n";
    //     return -1;
    // }


    try
    {
        umba::cli_tool_helpers::IoFileType inputFileType  = umba::cli_tool_helpers::IoFileType::nameEmpty;
        umba::cli_tool_helpers::IoFileType outputFileType = umba::cli_tool_helpers::IoFileType::nameEmpty;
        adjustInputOutputFilenames(inputFilename, inputFileType, outputFilename, outputFileType);

       
        bool utfSource = false;
        bool checkBom  = true;
        bool fromFile  = true;

        std::string srcData = umba::cli_tool_helpers::readInput( inputFilename , inputFileType
                            , encoding::ToUtf8(), checkBom, fromFile, utfSource //, outputLinefeed
                            );

        //  
        // // Есть ли данные на входе, нет их - это не наша проблема - процессим пустой текст в нормальном режиме

        std::string bomData = umba::cli_tool_helpers::stripTheBom(srcData, checkBom, encoding::BomStripper());
       
        //------------------------------
       
        // Do job itself
       
        ELinefeedType detectedSrcLinefeed = ELinefeedType::crlf;
       
        std::string lfNormalizedText = marty_cpp::normalizeCrLfToLf(srcData, &detectedSrcLinefeed);
       
        if (outputLinefeed==ELinefeedType::unknown || outputLinefeed==ELinefeedType::detect)
        {
            outputLinefeed = detectedSrcLinefeed;
        }

        #if defined(WIN32) || defined(_WIN32)
        if (outputFileType==umba::cli_tool_helpers::IoFileType::clipboard)
        {
            outputLinefeed = ELinefeedType::crlf;
        }
        #endif
       
       
        std::vector<std::string> textLines = marty_cpp::splitToLinesSimple( lfNormalizedText
                                                                          , true // addEmptyLineAfterLastLf
                                                                          , '\n' // lfChar
                                                                          );
       
        std::vector<std::string> sortedLines = marty_cpp::sortIncludes(textLines, sortIncludeOptions);
       
        std::string resultText = marty_cpp::mergeLines(sortedLines, outputLinefeed, false /* addTrailingNewLine */);
       
        //------------------------------

        umba::cli_tool_helpers::writeOutput( outputFilename, outputFileType
                                           , encoding::ToUtf8(), encoding::FromUtf8()
                                           , resultText, bomData
                                           , fromFile, utfSource, bOverwrite
                                           );

    } // try
    catch(const std::runtime_error &e)
    {
        LOG_ERR_OPT << e.what() << "\n";
    }

    return 0;
}

