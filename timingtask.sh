#!/bin/bash
dir=$(dirname $0)
config="${dir}/configure/device.config"
log="${dir}/log"
result=$(cat $config | grep "setting" |cut -d '=' -f2)
if [ $? -ne 0 ]
then
    echo "getting the facility state failed" >> $log
    exit 1
fi
if [ result=='2' ]
then
    /sbin/reboot
fi

