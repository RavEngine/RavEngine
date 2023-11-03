@echo OFF

set orig=%cd%
cd /d "%UserProfile%\AppData\Roaming"
for %%a in ("%orig%") do set dir=%%~nxa
mkdir %dir%_Win
cd %dir%_Win
cmake %orig%
explorer .
cd %orig%

