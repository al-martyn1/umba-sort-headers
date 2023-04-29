#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "encoding/encoding.h"

#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif

#include "encoding/encoding.h"

#include "umba/filesys.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace app_utils {




//----------------------------------------------------------------------------
inline
bool readStreamHelper(std::istream &fileIn, std::string &data)
{
    std::vector<char> buf;
    
    if (!umba::filesys::readFile(fileIn, buf))
        return false;

    data.clear();
    if (buf.empty())
        return true;

    data = std::string(&buf[0], buf.size());

    return true;
}

//----------------------------------------------------------------------------
inline
bool writeStreamHelper(std::ostream &fileOut, const std::string &data)
{
    auto dataSize = data.size();
    if (!dataSize)
        return true;

    return umba::filesys::writeFile(fileOut, data.data(), dataSize);
}

//----------------------------------------------------------------------------
inline
bool readFileHelper( const std::string &fileName, std::string &data )
{
    std::vector<char> buf;
    
    if (!umba::filesys::readFile(fileName, buf))
        return false;

    data.clear();
    if (buf.empty())
        return true;

    data = std::string(&buf[0], buf.size());

    return true;
}

//----------------------------------------------------------------------------
inline
bool writeFileHelper( const std::string &fileName, const std::string &data, bool bOverwrite )
{
    return umba::filesys::writeFile(fileName, data.data(), data.size(), bOverwrite);
}

//----------------------------------------------------------------------------
inline
bool isFileReadableHelper( const std::string &fileName )
{
    return umba::filesys::isFileReadable(fileName);
}

//----------------------------------------------------------------------------
/*
inline
std::string stripBom( std::string &text)
{
    encoding::EncodingsApi* pApi = encoding::getEncodingsApi();
   
    std::size_t bomSize = 0;
    //std::string detectRes = pApi->detect( text, bomSize );
    UINT cpId = pApi->checkTheBom( text.data(), text.size(), &bomSize );
   
    if (!cpId)
        std::string();

    if (bomSize)
    {
        std::string bom = std::string(text, 0, bomSize);
        text.erase(0,bomSize);
        return bom;
    }

    return std::string();
}
*/
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
enum class IoFileType
{
    nameEmpty,
    regularFile,
    //stdioFile, // minus/'-'
    stdinFile,
    stdoutFile,
    clipboard
};

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
IoFileType detectFilenameType(const std::string &n, bool bInput=false)
{
    if (n.empty())
        return IoFileType::nameEmpty;

    std::string N = marty_cpp::toUpper(n);

    if (N=="-")
    {
        return bInput ? IoFileType::stdinFile : IoFileType::stdoutFile;
        //return IoFileType::stdioFile;
    }

    if (N=="STDIN")
        return IoFileType::stdinFile;

    if (N=="STDOUT")
        return IoFileType::stdoutFile;

#if defined(HAS_CLIPBOARD_SUPPORT)
    if (N=="CLPB"|| N=="CLIPBRD" /* || N=="{CLIPBRD}" || N=="{CLIPBOARD}" */ )
        return IoFileType::clipboard;
#endif

    return IoFileType::regularFile;
};

//----------------------------------------------------------------------------
inline
bool checkIoFileType(IoFileType ioFt, std::string &msg, bool bInput=false)
{
    if (ioFt==IoFileType::nameEmpty)
    {
        msg = "no input file name taken";
        return false;
    }

    if (bInput)
    {
        if (ioFt==IoFileType::stdoutFile)
        {
            msg = "can't use STDOUT for input";
            return false;
        }
    }
    else // output file
    {
        if (ioFt==IoFileType::stdinFile)
        {
            msg = "can't use STDIN for output";
            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------
inline
std::string adjustOutputFilename(const std::string &outputFilename, const std::string &inputFilename, IoFileType inputFileType)
{
    if (!outputFilename.empty())
        return outputFilename;

    if (inputFileType==IoFileType::stdinFile)
        return "STDOUT";

    return inputFilename;
}

//----------------------------------------------------------------------------
#if defined(HAS_CLIPBOARD_SUPPORT)
inline
bool getClipboardTextHelper(std::string &text, bool *pUtf=0)
{
    #if defined(WIN32) || defined(_WIN32)
    std::wstring wstr;
    if (umba::win32::clipboardTextGet(wstr, umba::win32::clipboardGetConsoleHwnd()))
    {
        text = encoding::toUtf8(wstr);
        if (pUtf)
           *pUtf = true;
    }
    else if (umba::win32::clipboardTextGet(text, umba::win32::clipboardGetConsoleHwnd()))
    {
        if (pUtf)
           *pUtf = false;
    }
    else
    {
        return false;
    }

    return true;

    #else

    return false;

    #endif
}

//------------------------------
inline
bool setClipboardTextHelper(const std::string &text, bool utf)
{
    #if defined(WIN32) || defined(_WIN32)
    if (utf)
    {
        return umba::win32::clipboardTextSet(encoding::fromUtf8(text), umba::win32::clipboardGetConsoleHwnd());
    }
    else
    {
        return umba::win32::clipboardTextSet(text, umba::win32::clipboardGetConsoleHwnd());
    }
    #else

    return false;

    #endif
}

#endif
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
bool readFileHelper(IoFileType ioFt, const std::string &fileName, std::string &data)
{
    if (ioFt==IoFileType::stdinFile)
        return readStreamHelper(std::cin, data);

    return readFileHelper(fileName, data);
}

//----------------------------------------------------------------------------
inline
bool writeFileHelper(IoFileType ioFt, const std::string &fileName, const std::string &data, bool bOverwrite)
{
    if (ioFt==IoFileType::stdoutFile)
        return writeStreamHelper(std::cout, data);

    return writeFileHelper(fileName, data, bOverwrite);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace app_utils


