#!/bin/bash
#echo "hello">/home/linaro/project/rmfsystem_4_6/log
#/home/linaro/project/rmfsystem_4_6/main>>/home/linaro/project/rmfsystem_4_6/log  &

cur=$(dirname $0)
main="${cur}/main"
log="${cur}/log"
$main>>$log &
