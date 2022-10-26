SETLOCAL
rem replace the url below with your test url 
rem in the format of http://xxx.xxx.xxx.xxx:xxxx/xxxx.html
rem It is a good result with the output "找不到差异"

set url=http://www.baidu.com
set cmd1= curl %url%
start %cmd1% -o 1.txt
start %cmd1% -o 2.txt
start %cmd1% -o 3.txt
start %cmd1% -o 4.txt
start %cmd1% -o 5.txt
start %cmd1% -o 6.txt

ENDLOCAL
timeout /t 2 /nobreak > nul
fc 1.txt 2.txt
fc 2.txt 3.txt
fc 3.txt 4.txt
fc 4.txt 5.txt
fc 5.txt 6.txt
del 1.txt 2.txt 3.txt 4.txt 5.txt 6.txt
pause
