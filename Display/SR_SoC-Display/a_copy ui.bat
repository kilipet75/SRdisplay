@echo off
setlocal

REM Pfad zum Unterordner
set "ui_folder=ui"
set "exclude_prefix=ui_themes."


echo Ueberpruefen, ob ui.c im Unterordner existiert
if not exist "%ui_folder%\ui.c" (
    echo Datei "%ui_folder%\ui.c" nicht gefunden. Vorgang wird abgebrochen.
    pause
    exit /b 1
)

echo Alle Dateien, die mit ui beginnen, im Hauptordner loeschen
for %%F in (ui*.*) do (
    echo %%~nxF | find /i "%exclude_prefix%" >nul
    if errorlevel 1 (
        del /q "%%~F"
    )
)

echo Alle ui*.* Dateien aus dem Unterordner in den Hauptordner kopieren
for %%F in (%ui_folder%\ui*.*) do (
    echo %%~nxF | find /i "%exclude_prefix%" >nul
    if errorlevel 1 (
        copy /y "%%~F" . >nul
    )
)

echo Alle ui*.* Dateien im Unterordner loeschen
del /q "%ui_folder%\ui*.*"

echo Vorgang abgeschlossen.

endlocal
