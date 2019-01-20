@echo off
rem most of this is based on documentation from https://ss64.com/nt/for.html

rem %1 ~> platform target (x64, win32)
rem %2 ~> configuration (debug, release)
rem %3 ~> solution dir
rem %4 ~> target dir

mkdir %4
cd %3\djinn\shaders
for %%d in (*.glsl) do call :compile %%d %4

goto :end

:compile
echo glslangvalidator "%1" -V -o "%~2\%~n1.spv"
glslangvalidator "%1" -V -o "%~2\%~n1.spv"
exit /b

:end
