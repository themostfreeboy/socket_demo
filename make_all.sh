#!/bin/bash

SCRIPT_DIR=$(dirname $(readlink -f $0))

PARAM=$1

cd ${SCRIPT_DIR}
dir_list=$(ls -l | awk '/^d/ {print $NF}')
for dir_name in ${dir_list}
do
    cp -vf ${SCRIPT_DIR}/makefile ${SCRIPT_DIR}/${dir_name}/makefile
    if [ "${PARAM}" == "clean" ];then
        cd ${SCRIPT_DIR}/${dir_name} && make clean && rm -vf ${SCRIPT_DIR}/${dir_name}/makefile
    else
        cd ${SCRIPT_DIR}/${dir_name} && make
    fi
done
