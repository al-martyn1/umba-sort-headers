#pragma once

#include <stack>

//#include "app_config.h"
#include "umba/cmd_line.h"


#if defined(WIN32) || defined(_WIN32)
    #include <shellapi.h>
#endif

// AppConfig    appConfig;


struct ArgParser
{

std::stack<std::string> optFiles;


std::string makeAbsPath( std::string p )
{
    std::string basePath;

    if (optFiles.empty())
        basePath = umba::filesys::getCurrentDirectory<std::string>();
    else
        basePath = umba::filename::getPath(optFiles.top());


    return umba::filename::makeAbsPath( p, basePath );

}



// 0 - ok, 1 normal stop, -1 - error
template<typename ArgsParser>
int operator()( const std::string                               &a           //!< строка - текущий аргумент
              , umba::command_line::CommandLineOption           &opt         //!< Объект-опция, содержит разобранный аргумент и умеет отвечать на некоторые вопросы
              , ArgsParser                                      &argsParser  //!< Класс, который нас вызывает, содержит некоторый контекст
              , umba::command_line::ICommandLineOptionCollector *pCol        //!< Коллектор опций - собирает инфу по всем опциям и готов вывести справку
              , bool fBuiltin
              , bool ignoreInfos
              )
{
    //using namespace marty::clang::helpers;

    std::string dppof = "Don't parse predefined options from ";

    if (opt.isOption())
    {
        std::string errMsg;
        int intVal;
        //unsigned uintVal;
        std::size_t szVal;
        bool boolVal;

        if (opt.name.empty())
        {
            LOG_ERR_OPT<<"invalid (empty) option name\n";
            return -1;
        }

       if (opt.isOption("quet") || opt.isOption('q') || opt.setDescription("Operate quetly"))  // . Short alias for '--verbose=quet'
        {
            argsParser.quet = true;
            //appConfig.setOptQuet(true);
        }

        #if defined(WIN32) || defined(_WIN32)
        else if (opt.isOption("home") || opt.setDescription("Open homepage"))
        {
            if (argsParser.hasHelpOption) return 0;
            ShellExecuteA( 0, "open", appHomeUrl, 0, 0, SW_SHOW );
            return 1;
        }
        #endif

        // else if (opt.setParam("LEVEL", 1, "0/quet/no/q|" 
        //                                   "1/normal/n|" 
        //                                   "2/config/c|" 
        //                                   "3/detailed/detail/d|" 
        //                                   "4/extra/high/e" 
        //                      )
        //       || opt.setInitial(1) || opt.isOption("verbose") || opt.isOption('V')
        //       || opt.setDescription("Set verbosity level. LEVEL parameter can be one of the next values:\n"
        //                             "quet - maximum quet mode (same as --quet).\n"
        //                             "normal - print common details.\n"
        //                             "config - print common details and app config.\n"
        //                             "detailed - print common details, app config and all declarations, which are found in user files.\n"
        //                             "extra - print common details, app config and all found declarations (from all files)." // "\n"
        //                            )
        //       )
        // {
        //     if (argsParser.hasHelpOption) return 0;
        //  
        //     auto mapper = [](int i) -> VerbosityLevel
        //                   {
        //                       //return AppConfig::VerbosityLevel_fromStdString((VerbosityLevel)i);
        //                       switch(i)
        //                       {
        //                           case  0: case  1: case  2: case  3: case  4: return (VerbosityLevel)i;
        //                           default: return VerbosityLevel::begin;
        //                       }
        //                   };
        //  
        //     VerbosityLevel lvl;
        //     if (!opt.getParamValue( lvl, errMsg, mapper ) )
        //     {
        //         LOG_ERR_OPT<<errMsg<<"\n";
        //         return -1;
        //     }
        //  
        //     //appConfig.setVerbosityLevel(lvl);
        //     if (lvl==VerbosityLevel::quet)
        //         argsParser.quet = true;
        // }

        else if ( opt.isBuiltinsDisableOptionMain  () 
               || opt.setDescription( dppof + "main distribution options file '" + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::appGlobal   ) + "'"))
        { } // simple skip - обработка уже сделана

        else if ( opt.isBuiltinsDisableOptionCustom() 
               || opt.setDescription( dppof + "custom global options file '"     + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::customGlobal) + "'"))
        { } // simple skip - обработка уже сделана

        else if ( opt.isBuiltinsDisableOptionUser  () 
               || opt.setDescription( dppof + "user local options file '"        + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::userLocal   ) + "'"))
        { } // simple skip - обработка уже сделана

        else if (opt.isOption("version") || opt.isOption('v') || opt.setDescription("Show version info"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!ignoreInfos)
            {
                printOnlyVersion();
                return 1;
            }
        }

        else if (opt.isOption("where") || opt.setDescription("Show where the executable file is"))
        {
            if (argsParser.hasHelpOption) return 0;

            LOG_MSG_OPT << programLocationInfo.exeFullName << "\n";
            return 0;
        }

        else if (opt.setParam("CLR", 0, "no/none/file|" 
                                        "ansi/term|" 
                                        #if defined(WIN32) || defined(_WIN32)
                                        "win32/win/windows/cmd/console"
                                        #endif
                             )
              || opt.setInitial(-1) || opt.isOption("color") 
              || opt.setDescription("Force set console output coloring")
              /* ", can be:\nno, none, file - disable coloring\nansi, term - set ansi terminal coloring\nwin32, win, windows, cmd, console - windows console specific coloring method" */
              )
        {
            if (argsParser.hasHelpOption) return 0;

            umba::term::ConsoleType res;
            auto mapper = [](int i) -> umba::term::ConsoleType
                          {
                              switch(i)
                              {
                                  case 0 : return umba::term::ConsoleType::file;
                                  case 1 : return umba::term::ConsoleType::ansi_terminal;
                                  case 2 : return umba::term::ConsoleType::windows_console;
                                  default: return umba::term::ConsoleType::file;
                              };
                          };
            if (!opt.getParamValue( res, errMsg, mapper ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            coutWriter.forceSetConsoleType(res);
            cerrWriter.forceSetConsoleType(res);
        }

        else if ( opt.setParam("N", 1, 0, 8)
               || opt.isOption("group") || opt.isOption('G') 
               || opt.setDescription("Set number of lines between groups"))
        {
            if (argsParser.hasHelpOption) return 0;
         
            if (!opt.getParamValue( szVal, errMsg ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
         
            sortIncludeOptions.sepGroupLines = szVal;
         
            return 0;
        }

        else if ( opt.setParam("N", 1, 0, 8)
               || opt.isOption("block") || opt.isOption('B') 
               || opt.setDescription("Set number of lines between groups"))
        {
            if (argsParser.hasHelpOption) return 0;
         
            if (!opt.getParamValue( szVal, errMsg ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
         
            sortIncludeOptions.sepBlockLines = szVal;

            return 0;
        }

        else if ( opt.isOption("system") || opt.isOption("sys") || opt.isOption('S')
               || opt.setDescription("System headers first."))
            {
                if (argsParser.hasHelpOption) return 0;
                sortIncludeOptions.sysIncludesFirst = true;
                return 0;
            }

        else if ( opt.isOption("user") || opt.isOption('U')
               || opt.setDescription("Iser headers first."))
            {
                if (argsParser.hasHelpOption) return 0;
                sortIncludeOptions.sysIncludesFirst = false;
                return 0;
            }

        else if ( opt.setParam("?MODE",true)
               || opt.isOption("overwrite") || opt.isOption('Y') 
               // || opt.setParam("VAL",true)
               || opt.setDescription("Allow overwrite existing file"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(boolVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            bOverwrite = boolVal;
            return 0;
        }

        else if ( opt.isOption("autocomplete-install") 
               || opt.setDescription("Install autocompletion to bash"
                                     #if defined(WIN32) || defined(_WIN32)
                                         "/clink(cmd)"
                                     #endif
                                    )
               )
        {
            if (argsParser.hasHelpOption) return 0;

            //return autocomplete(opt, true);
            return umba::command_line::autocompletionInstaller( pCol, opt, pCol->getPrintHelpStyle(), true, [&]( bool bErr ) -> decltype(auto) { return bErr ? LOG_ERR_OPT : LOG_MSG_OPT; } );
        }
        else if ( opt.isOption("autocomplete-uninstall") 
               || opt.setDescription("Remove autocompletion from bash"
                                     #if defined(WIN32) || defined(_WIN32)
                                         "/clink(cmd)"
                                     #endif
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            //return autocomplete(opt, false);
            return umba::command_line::autocompletionInstaller( pCol, opt, pCol->getPrintHelpStyle(), false, [&]( bool bErr ) -> decltype(auto) { return bErr ? LOG_ERR_OPT : LOG_MSG_OPT; } );
        }

        else if (opt.isHelpStyleOption())
        {
            // Job is done in isHelpStyleOption
        }
        else if (opt.isHelpOption()) // if (opt.infoIgnore() || opt.isOption("help") || opt.isOption('h') || opt.isOption('?') || opt.setDescription(""))
        {
            if (!ignoreInfos)
            {
                if (pCol && !pCol->isNormalPrintHelpStyle())
                    argsParser.quet = true;
                //printNameVersion();
                if (!argsParser.quet)
                {
                    printBuildDateTime();
                    printCommitHash();
                    std::cout<<"\n";
                //printHelp();
                }

                if (pCol && pCol->isNormalPrintHelpStyle() && argsParser.argsNeedHelp.empty())
                {
                    auto helpText = opt.getHelpOptionsString();
                    std::cout << "Usage: " << programLocationInfo.exeName
                              << " [OPTIONS] input_file [output_file]\n"
                              << "  Use {STDIN}/{STDOUT} to use STDIN (for input) or STDOUT (for output)\n"
                              // << "  Use '-' (minus sign) or {STDIN}/{STDOUT} to use STDIN (for input) or STDOUT (for output)\n"
                              // << "    If '-' (minus sign) used for input/output file names, separate them with '--' from OPTIONS\n"
                              << "  Use '{CLPB}'/'{CLPBRD}'/'{CLIPBRD}'/'{CLIPBOARD}' to use clipboard for input or output\n"
                              << "  If 'output_file' not taken, 'input_file' will be used (or STDOUT if input is from STDIN)\n"
                              << "\nOptions:\n\n"
                              << helpText;
                              //<< " [OPTIONS] input_file [output_file]\n\nOptions:\n\n"<<helpText;
                }
                
                if (pCol) // argsNeedHelp
                    std::cout<<pCol->makeText( 78, &argsParser.argsNeedHelp );

                return 1;

            }

            return 0; // simple skip then parse builtins
        }
        else
        {
            LOG_ERR_OPT<<"unknown option: "<<opt.argOrg<<"\n";
            return -1;
        }

        return 0;

    } // if (opt.isOption())
    else if (opt.isResponseFile())
    {
        std::string optFileName = makeAbsPath(opt.name);

        optFiles.push(optFileName);

        auto parseRes = argsParser.parseOptionsFile( optFileName );

        optFiles.pop();

        if (!parseRes)
            return -1;

        if (argsParser.mustExit)
            return 1;

        return 0;
    
    }

    //appConfig.clangCompileFlagsTxtFilename.push_back(makeAbsPath(a));

    //appConfig.outputName = makeAbsPath(a);

    if (inputFilename.empty())
        inputFilename = a;
    else
        outputFilename = a;

    return 0;

}

}; // struct ArgParser



class CommandLineOptionCollector : public umba::command_line::CommandLineOptionCollectorImplBase
{
protected:
    virtual void onOptionDup( const std::string &opt ) override
    {
        LOG_ERR_OPT<<"Duplicated option key - '"<<opt<<"'\n";
        throw std::runtime_error("Duplicated option key");
    }

};


