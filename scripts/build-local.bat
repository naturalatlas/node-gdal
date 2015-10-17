@ECHO OFF
SETLOCAL
SET EL=0

ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SET PATH=C:\Python27;%PATH%

::SET APPVEYOR_REPO_COMMIT_MESSAGE=local build [publish binary] bla bla
::SET APPVEYOR_REPO_COMMIT_MESSAGE=local build [NOOOOO publish binary] bla bla
SET APPVEYOR_REPO_COMMIT_MESSAGE=hm? AppVeyor doesn't like "npm test || SET HAD_ERROR=1"? [publish binary]

IF EXIST lib\binding ECHO deleting lib/binding && RD /Q /S lib\binding
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
IF EXIST node_modules ECHO deleting node_modules && RD /Q /S node_modules
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO                 ============================
ECHO                           VS2013
ECHO                 ============================
SET nodejs_version=0.17.7
SET platform=x64
SET msvs_toolset=12
SET TOOLSET_ARGS=

::CALL scripts\build-appveyor.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR




IF EXIST lib\binding ECHO deleting lib/binding && RD /Q /S lib\binding
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
IF EXIST node_modules ECHO deleting node_modules && RD /Q /S node_modules
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO                 ============================
ECHO                           VS2015
ECHO                 ============================
::SET nodejs_version=4.2.1
SET nodejs_version=3.3.1
SET platform=x64
SET msvs_toolset=14
::SET TOOLSET_ARGS=--dist-url=https://s3.amazonaws.com/mapbox/node-cpp11 --toolset=v140

CALL scripts\build-appveyor.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR




GOTO DONE

:ERROR
ECHO ~~~~~~~~~~~~~~~~~~~~~~ ERROR %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE
ECHO ~~~~~~~~~~~~~~~~~~~~~~ DONE %~f0 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EXIT /b %EL%
