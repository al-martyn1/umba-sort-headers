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
#include "app_utils.h"


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


    if (inputFilename.empty())
    {
        inputFilename = "STDIN";
    }


    std::string checkMsg;

    app_utils::IoFileType inputFileType = app_utils::detectFilenameType(inputFilename, true /* input file */);
    if (!app_utils::checkIoFileType(inputFileType, checkMsg, true /* input file */))
    {
        LOG_ERR_OPT << checkMsg << "\n";
        return 1;
    }

    outputFilename = app_utils::adjustOutputFilename(outputFilename, inputFilename, inputFileType);
    app_utils::IoFileType outputFileType = app_utils::detectFilenameType(outputFilename, false /* not input file */);
    if (!app_utils::checkIoFileType(outputFileType, checkMsg, false /* not input file */))
    {
        LOG_ERR_OPT << checkMsg << "\n";
        return 1;
    }


    bool utfSource = false;
    bool checkBom  = true;
    bool fromFile  = true;
    std::string srcData;
    std::string bomData;

#if defined(HAS_CLIPBOARD_SUPPORT)
    if (inputFileType==app_utils::IoFileType::clipboard)
    {
        checkBom = false;
        fromFile = false;
        if (!app_utils::getClipboardTextHelper(srcData, &utfSource))
        {
            LOG_ERR_OPT << "failed to get clipboard text\n";
            return 1;
        }
        #if defined(WIN32) || defined(_WIN32)
        outputLinefeed = ELinefeedType::crlf;
        #endif
    }
    else
#endif
    if (!app_utils::readFileHelper(inputFileType, inputFilename, srcData))
    {
        LOG_ERR_OPT << "failed to read input file '" << inputFilename << "'\n";
        return 1;
    }


    // Есть ли данные на входе, нет их - это не наша проблема - процессим пустой текст в нормальном режиме

    if (checkBom)
    {
        bomData = encoding::getEncodingsApi()->stripTheBom(srcData); // app_utils::stripBom(srcData);
    }

    if (!bomData.empty() && bomData.size()!=3)
    {
        // https://ru.wikipedia.org/wiki/%D0%9C%D0%B0%D1%80%D0%BA%D0%B5%D1%80_%D0%BF%D0%BE%D1%81%D0%BB%D0%B5%D0%B4%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8C%D0%BD%D0%BE%D1%81%D1%82%D0%B8_%D0%B1%D0%B0%D0%B9%D1%82%D0%BE%D0%B2
        LOG_ERR_OPT << "unsupported input file encoding, found BOM but not UTF-8\n";
        return 1;
    }


    //------------------------------

    // Do job itself

    ELinefeedType detectedSrcLinefeed = ELinefeedType::crlf;

    std::string lfNormalizedText = marty_cpp::normalizeCrLfToLf(srcData, &detectedSrcLinefeed);

    if (outputLinefeed==ELinefeedType::unknown || outputLinefeed==ELinefeedType::detect)
    {
        outputLinefeed = detectedSrcLinefeed;
    }


    std::vector<std::string> textLines = marty_cpp::splitToLinesSimple( lfNormalizedText
                                                                      , true // addEmptyLineAfterLastLf
                                                                      , '\n' // lfChar
                                                                      );

    std::vector<std::string> sortedLines = marty_cpp::sortIncludes(textLines, sortIncludeOptions);

    std::string resultText = marty_cpp::mergeLines(sortedLines, outputLinefeed, false /* addTrailingNewLine */);

    //------------------------------


#if defined(HAS_CLIPBOARD_SUPPORT)
    if (outputFileType==app_utils::IoFileType::clipboard)
    {
        if (fromFile)
        {
            // Вход был прочитан из файла, хз какая кодировка
            // Но кодировка не поменялась в процессе сортировки, просто строки местами поменялись
            // Детектим кодировку и конвертим в UTF-8, присунув исходный BOM, если был
            resultText = encoding::toUtf8(bomData+resultText);
            utfSource  = true;
        }

        if (!app_utils::setClipboardTextHelper(resultText, utfSource))
        {
            LOG_ERR_OPT << "failed to set clipboard text\n";
            return 1;
        }
    }
    else
#endif
    if (!app_utils::writeFileHelper(outputFileType, outputFilename, resultText, bOverwrite))
    {
        LOG_ERR_OPT << "failed to write output file '" << outputFilename << "'\n";
        return 1;
    }


    return 0;
}

