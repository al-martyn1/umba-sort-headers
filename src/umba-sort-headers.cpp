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

#include "utf.h"
#include "utils.h"


#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


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

        if (N=="STDIN")
            return IoFileType::stdinFile;

        if (N=="STDOUT")
            return IoFileType::stdoutFile;

#if defined(HAS_CLIPBOARD_SUPPORT)
        if (N=="CLPB" /*  || N=="{CLPBRD}" || N=="{CLIPBRD}" || N=="{CLIPBOARD}" */ )
            return IoFileType::clipboard;
#endif

        return IoFileType::regularFile;
    };


    if (inputFilename.empty())
    {
        inputFilename = "STDIN";
    }


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
            outputFilename = "STDOUT";
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

#if defined(HAS_CLIPBOARD_SUPPORT)
    if (outputFileType==IoFileType::clipboard)
    {
        outputLinefeed = ELinefeedType::crlf;
    }
#endif


    bool utfSource = false;
    std::string srcData;
    bool checkBom = true;
    std::string bomData;

#if defined(HAS_CLIPBOARD_SUPPORT)
    if (inputFileType==IoFileType::clipboard)
    {
        std::wstring wstr;
        std::string  astr;
        #if defined(WIN32) || defined(_WIN32)
        if (umba::win32::clipboardTextGet(wstr, umba::win32::clipboardGetConsoleHwnd()) && !wstr.empty())
        {
            srcData = toUtf8(wstr);
            utfSource = true;
        }
        else if (umba::win32::clipboardTextGet(astr, umba::win32::clipboardGetConsoleHwnd()) && !astr.empty())
        {
            srcData = astr;
            utfSource = false;
        }
        else
        {
            LOG_ERR_OPT << "failed to get clipboard text or clipboard is empty\n";
            return 0;
        }

        checkBom = false;

        #endif
    }
    else
#endif
    if (inputFileType==IoFileType::stdinFile)
    {
        srcData = readStreamHelper(std::cin);
    }
    else if (inputFileType==IoFileType::regularFile)
    {
        srcData = readFileHelper(inputFilename);
        if (srcData.empty())
        {
            LOG_ERR_OPT << "input file is missing or empty\n";
            return 1;
        }
    }
    else
    {
        LOG_ERR_OPT << "unknown input file type\n";
    }

    if (checkBom)
    {
        bomData = stripBom(srcData);
    }

    if (!bomData.empty() && bomData.size()!=3)
    {
        // https://ru.wikipedia.org/wiki/%D0%9C%D0%B0%D1%80%D0%BA%D0%B5%D1%80_%D0%BF%D0%BE%D1%81%D0%BB%D0%B5%D0%B4%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8C%D0%BD%D0%BE%D1%81%D1%82%D0%B8_%D0%B1%D0%B0%D0%B9%D1%82%D0%BE%D0%B2
        LOG_ERR_OPT << "unsupported file encoding, found BOM but not UTF-8\n";
        return 1;
    }
    


    ELinefeedType detectedSrcLinefeed = ELinefeedType::crlf;

    std::string lfNormalizedText = marty_cpp::normalizeCrLfToLf(srcData, &detectedSrcLinefeed);

    if (outputLinefeed==ELinefeedType::unknown || outputLinefeed==ELinefeedType::detect)
    {
        outputLinefeed = detectedSrcLinefeed;
    }

#if defined(HAS_CLIPBOARD_SUPPORT)
    if (outputFileType==IoFileType::clipboard)
    {
        #if defined(WIN32) || defined(_WIN32)
        outputLinefeed = ELinefeedType::crlf; // Для виндового клипборда - перевод строки всегда CRLF
        #endif
    }
#endif


    std::vector<std::string> textLines = marty_cpp::splitToLinesSimple( lfNormalizedText
                                                                      , true // addEmptyLineAfterLastLf
                                                                      , '\n' // lfChar
                                                                      );

    std::vector<std::string> sortedLines = marty_cpp::sortIncludes(textLines, sortIncludeOptions);

    std::string resultText = marty_cpp::mergeLines(sortedLines, outputLinefeed, false /* addTrailingNewLine */);

#if defined(HAS_CLIPBOARD_SUPPORT)
    if (outputFileType==IoFileType::clipboard)
    {
        #if defined(WIN32) || defined(_WIN32)
        if (utfSource)
        {
            std::wstring wText = fromUtf8(resultText);
            if (!umba::win32::clipboardTextSet(wText, umba::win32::clipboardGetConsoleHwnd()))
            {
                LOG_ERR_OPT << "failed to set clipboard text\n";
                return 1;
            }
        }
        else
        {
            if (!umba::win32::clipboardTextSet(resultText, umba::win32::clipboardGetConsoleHwnd()))
            {
                LOG_ERR_OPT << "failed to set clipboard text\n";
                return 1;
            }
        }
        #endif
    }
    else
#endif
    if (outputFileType==IoFileType::stdoutFile)
    {
        writeStreamHelper(std::cout, bomData+resultText);
    }
    else if (outputFileType==IoFileType::regularFile)
    {
        if (!writeFileHelper(outputFilename, bomData+resultText, bOverwrite))
        {
            LOG_ERR_OPT << "filed to write output file type\n";
        }
    }
    else
    {
        LOG_ERR_OPT << "unknown output file type\n";
        return 1;
    }

    return 0;
}

