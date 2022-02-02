#!/bin/sh
sed -i "s|path|$1|g" $1/WEAPP.desktop 
sed -i "s|path|$1|g" $1/WEAPP_script.sh 
