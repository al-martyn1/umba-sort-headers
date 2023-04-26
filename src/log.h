#pragma once

// requires global vars 
// logGccFormat
// logSourceInfo

//struct GeneratorOptions;
// if (generatorOptions.allowWarn("warn"))

// #include "rdlc-core/log.h"

enum class LogEntryType
{
    err, warn, msg
};



//template<typename GeneratorOptions>
inline
umba::SimpleFormatter& startLogError( umba::SimpleFormatter   &s
                                    , LogEntryType             logEntryType
                                    // , const GeneratorOptions &gopts
                                    , const std::string       &warnType
                                    , const char* inputFile   , unsigned inputLineNo
                                    , const char* srcFile = 0 , unsigned srcLineNo = 0
                                    )
{
    // if (logEntryType==LogEntryType::warn  /* && !gopts.allowWarn(warnType) */ )
    //     return logNul;
    //  

    using namespace umba::omanip;

    if (logEntryType==LogEntryType::err || logEntryType==LogEntryType::warn)
    {
        if (logEntryType==LogEntryType::err)
            s<<error;
        else
            s<<warning;
    }

    if (inputFile)
    {
        if (logGccFormat)
        {
            // ..\src\main\payloads_bus_master_task.h:60:1: error:
            if (inputFile) //-V547
                s<<inputFile<<":"<<inputLineNo<<": ";
        }
        else
        {
            // e:\_work\utils\rdlc\src\rdlc.cpp(668): error C2065:
            if (inputFile) //-V547
                s<<inputFile<<"("<<inputLineNo<<"): ";
        }

        if (logEntryType==LogEntryType::err)
            s<< "error:";
        else if (logEntryType==LogEntryType::warn)
        {
            s<< "warning";
            if (logWarnType)
                s<< "("<<warnType<<"):";
        }
    }
    else
    {
        if (logEntryType==LogEntryType::err)
            s<< "error:";
        else if (logEntryType==LogEntryType::warn)
        {
            s<< "warning";
            if (logWarnType)
                s<< "("<<warnType<<"):";
        }
    }

    if (srcFile && logSourceInfo)
    {
        s<<" ";
        s<<coloring( UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::blue, umba::term::colors::color_default, true  ,  false,  false ) )
         <<"["<<srcFile<<":"<<srcLineNo<<"]";
    }
    
    if (logEntryType==LogEntryType::err || logEntryType==LogEntryType::warn)
        s<<normal<<" ";

    return s;
}

/*
inline
umba::SimpleFormatter& startLogError( umba::SimpleFormatter &s, bool bWarning
                                    , const std::string const char* inputFile, unsigned inputLineNo
                                    , const char* srcFile = 0, unsigned srcLineNo = 0
                                    )
{
    return startLogError( umba::SimpleFormatter &s, bool bWarning
                                    , const char* inputFile, unsigned inputLineNo
                                    , const char* srcFile = 0, unsigned srcLineNo = 0
                                    )
}
*/

// source parsing errors
#define LOG_ERR                 startLogError( logErr, LogEntryType::err ,  /* gopts, */  std::string("err")    , curFile.c_str(), lineNo, __FILE__, __LINE__ )
#define LOG_WARN(warnType)      startLogError( logErr, LogEntryType::warn,  /* gopts, */  std::string(warnType) , curFile.c_str(), lineNo, __FILE__, __LINE__ )

// options and other errors
#define LOG_ERR_OPT             startLogError( logErr, LogEntryType::err ,  /* gopts,  */ std::string("err")    , (const char*)0 , 0     , __FILE__, __LINE__ )
#define LOG_WARN_OPT(warnType)  startLogError( logErr, LogEntryType::warn,  /* gopts,  */ std::string(warnType) , (const char*)0 , 0     , __FILE__, __LINE__ )
                                                                            /*         */                                        
#define LOG_MSG_OPT             startLogError( logMsg, LogEntryType::msg ,  /* gopts,  */ std::string("msg")    , (const char*)0 , 0     , (const char*)0, 0 )


