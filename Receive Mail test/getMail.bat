@echo off
if not "%minimized%"=="" goto :minimized
set minimized=true
start /min cmd /c "%~dpnx0"
goto :EOF
:minimized
curl -v pop3s://pop.gmail.com:995/9 --ssl-reqd ^
  --connect-timeout 20 ^
  --max-time 60 ^
  -u "ductai.dt05@gmail.com:gofe zjfi ktmw fxno" ^
  -o latest_email.eml
