@call :DO_GIT_JOB %1 marty_cpp
@call :DO_GIT_JOB %1 umba
@call :DO_GIT_JOB %1 sfmt


@exit /B


:DO_GIT_JOB
@echo %1'ing %2
@cd %2
@git %1
@cd ..
@echo.
@exit /B

