:loop
set pa=%cd%
C:
cd C:\Users\ywf\AppData\Local\Google\Chrome\Application
chrome.exe --enable-logging --v=1
cd ../User Data
echo f|xcopy chrome_debug.log %pa%\chrome_debug.log /Y/K 

pause 
goto loop