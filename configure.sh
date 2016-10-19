#!/bin/bash

#user can use this script to configure the RMF device 


echo "1.set HDD............"
echo "1.1 create HDD......."
echo  "please input the HDD device:"
read -p "#>" disk
echo "Are you sure the device: " $disk
echo ":1 yes 2 no"
read -p "#>" result
if [ $result != "1"  ]
then
    echo "please reexecute the shell script......"
    exit 0
fi 
echo -e "n\np\n1\n\n\nw\n" | fdisk $disk &>/dev/null
echo "create HDD successfully......"
echo "1.2 format HDD............"
echo -e "y\n" | mkfs.ext3 $disk
if [ $? -eq 0  ]
then 
    echo "foramt HDD successfully......"
fi
echo "1.3 mount HDD............"
echo "establish the database dictionary: /data"
mkdir /data
if [ $? -eq 0  ]
then
    echo "establish the dictionary successfully......"
fi
echo "execute the mount operation"
mount="${disk} /data ext3 defaults 0 0"
echo $mount >> /etc/fstab
tail -n 1 /etc/fstab
echo "Are you sure the content?"
echo ":1 yes 2 no"
read -p "#>" result
if [ $result != "1" ]
then
    echo "please reexecute the shell script......"
    exit 0
fi
echo "2.set time............"
echo "the local time is : ",$(date -R)
echo "2.1 set the time zone........."
echo "the default time zone is GMT+8"
echo ":1 yes 2 no"
read -p "#>" result
if [ $result != "2"  ]
then 
    echo "TZ='Asia/Shanghai';export TZ" >>/home/linaro/.profile
    source /home/linaro/.profile
fi
echo "setting the timezone successfully......"
echo "The current time is :",$(date -R)
echo "Do you want to change the time?"
echo ":1 yes 2 no"
read -p "#>" result
if [ $result != "2" ]
then
    echo "set the time.........."
    echo "first set the date:the format like 2016-05-31"
    read -p "#>" datetime
    date -s $datetime
    echo "then set the time:the format like 14:15:00"
    read -p "#>" daytime
    date -s $daytime
    echo "the current time is:",$(date -R)
fi
echo "3.install library support........."
echo "3.1 update the system.........."
apt-get update
echo "3.2 install sqlite3 support......"
apt-get install sqlite3 sqlite libsqlite3-dev
echo "3.3 install xmlprase support........."
apt-get install libxml2 libxml2-dev
echo "3.4 install python support........."
apt-get install python-dev
echo "4. compile the programme......"
echo "4.1 makefile........."
make clean
make
echo "4.2 makeclean........."
make clean
echo "5.change the profile........."
echo "first select the plc type......"
echo -e "1:s7-300\n2:ABB\n3:schneider\n4:etc..."
read -p "#>" plctype
case "$plctype" in
    1) echo "you select s7-300"
        echo "please input the block:"
        echo -e "1.DB\n2.M\n3.I\n4.Q\n"
        read -p "#>" block
        case "$block" in
            1) echo "you select DB"
            sed -i "s/^\(org_id=\).*/\101/g" configure/device.config
            ;;
            2) echo "you select M"
            sed -i "s/^\(org_id=\).*/\102/g" configure/device.config
            ;;
            3) echo "you select I"
            sed -i "s/^\(org_id=\).*/\103/g" configure/device.config
            ;;
            4) echo "you select Q"
            sed -i "s/^\(org_id=\).*/\104/g" configure/device.config
            ;;
        esac
        echo "please input the DB block number"
        read -p "#>" db_block
        sed -i "s/^\(area=\).*/\1${db_block}/g" configure/device.config
        echo "please input the start address:"
        echo -e "the format should be 16 binary system and 2 bytes length\nlike: 0000"
        read -p "#>" start_address
        start_address_h=${start_address:0:2}
        start_address_l=${start_address:2:4}
        sed -i "s/^\(start_address_h=\).*/\1${start_address_h}/g" \
        configure/device.config
        sed -i "s/^\(start_address_l=\).*/\1${start_address_l}/g" \
        configure/device.config
        echo "please input the data length:"
        echo -e "the format should be 16 binary system and 2 bytes length\nlike: 0000"
        read -p "#>" data_length
        data_length_h=${data_length:0:2}
        data_length_l=${data_length:2:4}
        sed -i "s/^\(len_h=\).*/\1${data_length_h}/g" \
        configure/device.config
        sed -i "s/^\(len_l=\).*/\1${data_length_l}/g" \
        configure/device.config
        echo "please input the normal TCP collection rate (us)"
        read -p "#>" collection_rate
        sed -i "s/^\(fast_time=\).*/\1${collection_rate}/g" \
        configure/device.config
        echo "please input the slow TCP collection rate (s)"
        read -p "#>" collection_rate
        sed -i "s/^\(slow_time=\).*/\1${collection_rate}/g" \
        configure/device.config
        echo -e "please confirm whether enable the power_off value"
        echo ":1.yes 2.no"
        read -p "#>" result
        if [ ${result} == "1" ]
        then
            sed -i "s/^\(running=\).*/\11/g" \
            configure/device.config
            echo "please input the length of the power-off value (bit)"
            read -p "#>" poweroff_len
            sed -i "s/^\(datalength=\).*/\1${poweroff_len}/g" \
            configure/device.config
            echo "please input the location of the power-off value "
            read -p "#>" poweroff_location
            sed -i "s/^\(datalocation=\).*/\1${poweroff_location}/g"  \
            configure/device.config
            echo "please input the offset of the power-off value "
            read -p "#>" poweroff_offset
            sed -i "s/^\(dataoffset=\).*/\1${poweroff_offset}/g" \
            configure/device.config
       else
           sed -i "s/^\(running=\).*/\10/g" \
           configure/device.config
       fi
    ;;
    2) echo "you select ABB"
    ;;
    3) echo "you select schneider"
    ;;
    4) echo "you select no"
    ;;
esac

    
echo "6.ip address setting........."
echo "6.1 PLC ipaddress setting......"
read -p "#>" plc_address
sed -i "s/^\(plcaddress=\).*/\1${plc_address}/g" configure/device.config
echo "6.2 server ipaddress setting......"
read -p "#>" server_address
sed -i "s/^\(servaddress=\).*/\1${server_address}/g" configure/device.config
echo "6.3 router ipaddress setting......"
read -p "#>" router_address
sed -i "s/^\(router_address=\).*/\1${router_address}/g" configure/device.config
echo "router router username setting......"
read -p "#>" username
sed -i "s/^\(username=\).*/\1${username}/g" configure/device.config
echo "router router password setting......"
read -p "#>" password
sed -i "s/^\(password=\).*/\1${password}/g" configure/device.config
echo "successfully........."
echo "6.5 RMFD ipaddress setting......"
read -p "#>" rmf_address
echo "set netmask......"
read -p "#>" netmask
echo -e "auto eth0\niface eth0 inet static\naddress ${rmf_address}\nnetmask ${netmask}\ngateway ${router_address}\n" >> /etc/network/interfaces

echo "7. autostart setting........."
echo "7.1 add executive right to the startup script........."
chmod +x startup.sh
echo "7.2 add autostart to the start-up file......... "
line=$(sed -n '/exit 0/=' /etc/rc.local |sed -n "2"p)
sed -i "${line}i\\$(pwd)/startup.sh"  /etc/rc.local

echo "8.set device information......"
echo "please input the username"
read -p "#>" dev_username
sed -i "s/^\(username=\).*/\1${dev_username}/g" configure/config
echo "please input the serialnumber"
read -p "#>" dev_serialnumber
sed -i "s/^\(serialnumber=\).*/\1${dev_serialnumber}/g" configure/config
echo "setting dev info ok!......"

echo "9.establish the database......"
echo "9.1 copy the sql script to /data direction......"
cp schema.sql /data
if [ $? -ne 0  ]
then 
    echo  "    copy the file to /data direction fail"
    exit 0
fi
echo "9.2 initialize the database......"
echo  -e ".read schema.sql" | sqlite3 /data/local.db &>/dev/null
if [ $? -ne 0  ]
then
    echo "    cannot initialize the database"
    exit 0
fi

echo "configure successfully........."

#echo "success"
