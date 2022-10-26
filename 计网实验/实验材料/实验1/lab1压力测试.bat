SETLOCAL
rem 压力测试，替换(1,1,100)中的100为测试数
rem url替换为http://ip:port/index.html

set url=http://www.baidu.com
set cmd1= curl %url%

for /L %%i in (1,1,100) do %cmd1% 
ENDLOCAL
pause
