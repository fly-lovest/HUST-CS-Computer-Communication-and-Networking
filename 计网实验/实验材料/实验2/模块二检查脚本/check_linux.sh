#!/bin/bash
# appname 程序名称
# inputname 输入文件名
# outputname 输出文件名
# resultname 程序控制台输出结果重定向文件名

appname='stop_wait'
inputname='input.txt'
outputname='output.txt'
resultname='result.txt'

for ((i=1;i<=10;i++))
do
echo Test $appname $i
./$appname > $resultname 2>&1
cmp $inputname $outputname
echo Test $i over
done
