#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "encoding/encoding.h"


// template<typename StringType>
inline
std::string readStreamHelper(std::istream &fileIn)
{
    std::string data;
    char buffer[4096];
    while (fileIn.read(buffer, sizeof(buffer)))
    {
        data.append(buffer, sizeof(buffer));
    }
    data.append(buffer, fileIn.gcount());
    
    return data;
}

inline
bool writeStreamHelper(std::ostream &fileOut, const std::string &data)
{
    if (!fileOut.write(data.data(), data.size()))
    {
        return false;
    }

    return true;
}


inline
std::string readFileHelper( const std::string &fileName )
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
            return std::string();
        }

        std::string data;

        std::vector<std::string::value_type> buf = std::vector<std::string::value_type>(16384,0); // 16Kb filled with zero

        DWORD numberOfBytesRead = 0;
        while(ReadFile(hFile, (LPVOID)&buf[0], buf.size(), &numberOfBytesRead, 0 ) && numberOfBytesRead)
        {
            data.append(buf.begin(), buf.begin()+numberOfBytesRead);
            numberOfBytesRead = 0;
        }

        CloseHandle(hFile);
       
        return data;

    #else

        std::ifstream fileIn;
        fileIn.open(fileName, std::ios_base::in | std::ios_base::binary);
        if (!fileIn)
        {
            return std::string();
        }

        return readStreamHelper(fileIn);

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
   
    size_t bomSize = 0;
    std::string detectRes = pApi->detect( text, bomSize );
   
    if (bomSize)
    {
        std::string bom = std::string(text, 0, bomSize);
        text.erase(0,bomSize);
        return bom;
    }

    return std::string();
}




