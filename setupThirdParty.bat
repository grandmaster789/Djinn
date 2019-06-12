@echo off
git submodule update

cd ThirdParty

mkdir include

mkdir lib
mkdir lib\x64
mkdir lib\x86

mkdir bin
mkdir bin\x64
mkdir bin\x86

@echo absl:
xcopy /y /d /s /i .\submodules\abseil\absl             include\absl

@echo concurrent queue:
xcopy /y /d    /i .\submodules\concurrentqueue\*.h     include

@echo cpp-std-fwd:
xcopy /y /d /s /i .\submodules\cpp-std-fwd\include     include

@echo glm:
xcopy /y /d /s /i .\submodules\glm\glm                 include\glm

@echo wil:
xcopy /y /d /s /i .\submodules\wil\include\wil         include\wil