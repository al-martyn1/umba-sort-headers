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

#include "utf.h"


namespace app_utils {


// template<typename StringType>
inline
bool readStreamHelper(std::istream &fileIn, std::string &data)
{
    //std::string data;
    data.clear();

    char buffer[4096];
    while (fileIn.read(buffer, sizeof(buffer)))
    {
        data.append(buffer, sizeof(buffer));
    }
    data.append(buffer, fileIn.gcount());
    
    return true;
}

inline
bool writeStreamHelper(std::ostream &fileOut, const std::string &data)
{
    auto dataSize = data.size();
    if (!fileOut.write(data.data(), dataSize))
    {
        return false;
    }

    return true;
}


inline
bool readFileHelper( const std::string &fileName, std::string &data )
{

    #if defined(WIN32) || defined(_WIN32)

        HANDLE hFile = CreateFileA( fileName.c_str()
                                  , GENERIC_READ    // dwDesiredAccess
                                  , FILE_SHARE_READ // dwShareMode
                                  , 0 // lpSecurityAttributes
                                  , OPEN_EXISTING // dwCreationDisposition
                                  , FILE_ATTRIBUTE_NORMAL // dwFlagsAndAttributes
                                  , 0 // hTemplateFile
                                  );
        if (hFile==INVALID_HANDLE_VALUE)
        {
            return false; // std::string();
        }

        // std::string data;
        data.clear();

        std::vector<std::string::value_type> buf = std::vector<std::string::value_type>(16384,0); // 16Kb filled with zero

        DWORD numberOfBytesRead = 0;
        while(ReadFile(hFile, (LPVOID)&buf[0], buf.size(), &numberOfBytesRead, 0 ) && numberOfBytesRead)
        {
            data.append(buf.begin(), buf.begin()+numberOfBytesRead);
            numberOfBytesRead = 0;
        }

        CloseHandle(hFile);
       
        return true;

    #else

        std::ifstream fileIn;
        fileIn.open(fileName, std::ios_base::in | std::ios_base::binary);
        if (!fileIn)
        {
            return false; // std::string();
        }

        return readStreamHelper(fileIn, data);

        // std::string data;
        // char buffer[4096];
        // while (fileIn.read(buffer, sizeof(buffer)))
        // {
        //     data.append(buffer, sizeof(buffer));
        // }
        // data.append(buffer, fileIn.gcount());
        //  
        // return data;

    #endif

}


inline
bool writeFileHelper( const std::string &fileName, const std::string &data, bool bOverwrite )
{

    #if defined(WIN32) || defined(_WIN32)

        DWORD dwCreationDisposition = bOverwrite ? CREATE_ALWAYS : CREATE_NEW;

        HANDLE hFile = CreateFileA( fileName.c_str()
                                  , GENERIC_WRITE    // dwDesiredAccess
                                  , FILE_SHARE_READ  // dwShareMode
                                  , 0 // lpSecurityAttributes
                                  , dwCreationDisposition
                                  , FILE_ATTRIBUTE_NORMAL // dwFlagsAndAttributes
                                  , 0 // hTemplateFile
                                  );
        if (hFile==INVALID_HANDLE_VALUE)
        {
            return false;
        }

        bool res = true;

        DWORD written = 0;
        if (!WriteFile(hFile, (LPVOID)data.data(), data.size(), &written, 0 ))
        {
            res = false;
        }

        if (written!=(DWORD)data.size())
        {
            res = false;
        }

        CloseHandle(hFile);
       
        return res;

    #else

        if (!bOverwrite) // не перезапись
        {
            std::ifstream fileIn;
            fileIn.open(fileName, std::ios_base::in | std::ios_base::binary);
            if (fileIn) // получилось открыть на чтение - существует
            {
                return false;
            }
        }

        std::ofstream fileOut;
        std::ios_base::openmode openMode = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
        fileOut.open(fileName, openMode);
        if (!fileOut)
        {
            return false;
        }

        return writeStreamHelper(fileOut, data);

    #endif

}



inline
bool isFileReadableHelper( const std::string &fileName )
{
    #if defined(WIN32) || defined(_WIN32)

        HANDLE hFile = CreateFileA( fileName.c_str()
                                  , GENERIC_READ    // dwDesiredAccess
                                  , FILE_SHARE_READ // dwShareMode
                                  , 0 // lpSecurityAttributes
                                  , OPEN_EXISTING // dwCreationDisposition
                                  , FILE_ATTRIBUTE_NORMAL // dwFlagsAndAttributes
                                  , 0 // hTemplateFile
                                  );
        if (hFile==INVALID_HANDLE_VALUE)
        {
            return false;
        }

        CloseHandle(hFile);

        return true;

    #else

        std::ifstream fileIn;
        fileIn.open(fileName, std::ios_base::in);
        if (!fileIn)
        {
            return false;
        }

        return true;

    #endif
}


inline
std::string stripBom( std::string &text)
{
    EncodingsApi* pApi = getEncodingsApi();
   
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

inline
std::string adjustOutputFilename(const std::string &outputFilename, const std::string &inputFilename, IoFileType inputFileType)
{
    if (!outputFilename.empty())
        return outputFilename;

    if (inputFileType==IoFileType::stdinFile)
        return "STDOUT";

    return inputFilename;
}


#if defined(HAS_CLIPBOARD_SUPPORT)
inline
bool getClipboardTextHelper(std::string &text, bool *pUtf=0)
{
    #if defined(WIN32) || defined(_WIN32)
    std::wstring wstr;
    if (umba::win32::clipboardTextGet(wstr, umba::win32::clipboardGetConsoleHwnd()))
    {
        text = toUtf8(wstr);
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

inline
bool setClipboardTextHelper(const std::string &text, bool utf)
{
    #if defined(WIN32) || defined(_WIN32)
    if (utf)
    {
        return umba::win32::clipboardTextSet(fromUtf8(text), umba::win32::clipboardGetConsoleHwnd());
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




inline
bool readFileHelper(IoFileType ioFt, const std::string &fileName, std::string &data)
{
    if (ioFt==IoFileType::stdinFile)
        return readStreamHelper(std::cin, data);

    return readFileHelper(fileName, data);
}

inline
bool writeFileHelper(IoFileType ioFt, const std::string &fileName, const std::string &data, bool bOverwrite)
{
    if (ioFt==IoFileType::stdoutFile)
        return writeStreamHelper(std::cout, data);

    return writeFileHelper(fileName, data, bOverwrite);
}



} // namespace app_utils


