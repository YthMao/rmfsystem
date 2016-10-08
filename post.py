#!/usr/bin/env 
#coding=utf-8
import urllib
import urllib2
import base64
import os
import re

ipsec_enable={
    'submit_button':'ipsec',
    'conn_count':'1',
    'mod_operation':'service',
    'h_status':'',
    'h_name':'',
    'h_auth':'',
    'h_protocol':'',
    'h_encrypt':'',
    'h_ipsec_encrypt':'',
    'h_key':'',
    'h_way':'',
    'h_consult':'',
    'h_interface':'',
    'h_subnet':'',
    'h_rip':'',
    'h_rsubnet':'',
    'h_adv_trsmode':'',
    'h_adv_id':'',
    'h_adv_rid':'',
    'h_adv_dh':'',
    'h_pfs_dh':'',
    'h_adv_pfs':'',
    'h_adv_ikelife':'',
    'h_adv_keylife':'',
    'h_adv_dpdstat':'',
    'h_adv_dpdact':'',
    'h_adv_dpdelay':'',
    'h_adv_dpdtimeout':'',
    'ipsec_service':'enable'
}

ipsec_disable={
    'submit_button':'ipsec',
    'conn_count':'1',
    'mod_operation':'service',
    'h_status':'',
    'h_name':'',
    'h_auth':'',
    'h_protocol':'',
    'h_encrypt':'',
    'h_ipsec_encrypt':'',
    'h_key':'',
    'h_way':'',
    'h_consult':'',
    'h_interface':'',
    'h_subnet':'',
    'h_rip':'',
    'h_rsubnet':'',
    'h_adv_trsmode':'',
    'h_adv_id':'',
    'h_adv_rid':'',
    'h_adv_dh':'',
    'h_pfs_dh':'',
    'h_adv_pfs':'',
    'h_adv_ikelife':'',
    'h_adv_keylife':'',
    'h_adv_dpdstat':'',
    'h_adv_dpdact':'',
    'h_adv_dpdelay':'',
    'h_adv_dpdtimeout':'',
    'ipsec_service':'disable'
}

def post_enable():
    pre_path=os.getcwd()
    path=os.path.join(pre_path,"device.config")
    try:
        fp=open(path,'r')
    except IOError,e:
        print Exception,":",e
    for eachline in fp:
        if "router_address" in eachline:
            router_address=eachline.split('=')[1][:-1]
        if "username" in eachline:
            username=eachline.split('=')[1][:-1]
        if "password" in eachline:
            password=eachline.split('=')[1][:-1]
    fp.close()
    data_urlencode=urllib.urlencode(ipsec_enable)
    requrl="http://" + router_address+"/apply.cgi"
#    print requrl
    req=urllib2.Request(url=requrl,data=data_urlencode)
    base_data=username+":"+password
 #   print base_data
    auth='Basic ' + base64.urlsafe_b64encode(base_data)
  #  print auth
    req.add_header('Authorization',auth)
    urllib2.urlopen(req)

def post_disable():
    pre_path=os.getcwd()
    path=os.path.join(pre_path,"device.config")
    try:
        fp=open(path,'r')
    except IOError,e:
        print Exception,":",e
    for eachline in fp:
        if "router_address" in eachline:
            router_address=eachline.split('=')[1][:-1]
        if "username" in eachline:
            username=eachline.split('=')[1][:-1]
        if "password" in eachline:
            password=eachline.split('=')[1][:-1]
    fp.close()
    data_urlencode=urllib.urlencode(ipsec_disable)
    requrl="http://" + router_address+"/apply.cgi"
    req=urllib2.Request(url=requrl,data=data_urlencode)
    base_data=username+":"+password
    auth='Basic ' + base64.urlsafe_b64encode(base_data)
    req.add_header('Authorization',auth)
    urllib2.urlopen(req)

if __name__=='__main__':
    post_enable()
