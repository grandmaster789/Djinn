@echo off

@rem This will apply clang format to all .cpp, .h and .inl files in all djinn and bazaar subfolders;
@rem would like to add djinn-test as well, but my current formatting rules don't work well with (unit-testing) macro's

@rem NOTE developed for clang-format version 8.0.0
@rem NOTE when installing the Clang power tools extension for MSVC, 
@rem      for me (ymmv) the format binary ended up at:
@rem      "%appdata%\..\Local\Microsoft\VisualStudio\16.0_9ca5a9df\Extensions\4yequj44.hvd"

cd djinn
  for /R %%d in (*.cpp) do call :format "%%d"
  for /R %%d in (*.h)   do call :format "%%d"
  for /R %%d in (*.inl) do call :format "%%d"
cd..

cd bazaar
  for /R %%d in (*.cpp) do call :format "%%d"
  for /R %%d in (*.h)   do call :format "%%d"
  for /R %%d in (*.inl) do call :format "%%d"
cd..

goto :end

:display
echo %1
goto :end

:format
echo %1
"%appdata%\..\Local\Microsoft\VisualStudio\16.0_9ca5a9df\Extensions\4yequj44.hvd\clang-format.exe" -i %1
goto :end

:end