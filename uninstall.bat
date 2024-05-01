@echo off
@rem ========================================
@rem Uninstall config ini file
@rem ========================================

set TARGET_NAME=ssisoroadgl
set TARGET_DIR=%APPDATA%\%TARGET_NAME%
set TARGET_INI=%TARGET_DIR%\%TARGET_NAME%.ini

echo Uninstall config file.
echo.
echo Target ini file. [%TARGET_INI%]
echo Target folder    [%TARGET_DIR%]
echo.

if exist %TARGET_INI% (
    echo Found ini file. delete.
    del %TARGET_INI%
) else (
    echo Not found ini file.
)

if exist %TARGET_DIR% (
    echo Found folder. delete.
    rd %TARGET_DIR%
) else (
    echo Not found folder.
)

pause
