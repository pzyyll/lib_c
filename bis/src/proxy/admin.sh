#!/bin/sh

ulimit -c unlimited 

SERVER_NAME="proxy"
SERVER_BIN_PATH="./"
SERVER_PARAM="./${SERVER_NAME}.ini"

start()
{
        ${SERVER_BIN_PATH}/${SERVER_NAME} ${SERVER_PARAM} 
        if [ $? -eq 0 ];then
                echo "Start server ${SERVER_NAME} OK"
        else
                echo "Start server ${SERVER_NAME} FAILED"
        fi
}

stop()
{
        killall ${SERVER_NAME}
        usleep 1000000
        proc_num=$(ps -ef|grep ${SERVER_NAME}|grep -v grep|wc -l)
        if [ ${proc_num} -gt 0 ];then
                killall -9 ${SERVER_NAME}
        fi

        proc_num=$(ps -ef|grep ${SERVER_NAME}|grep -v grep|wc -l)
        if [ ${proc_num} -gt 0 ];then
                echo "Stop server ${SERVER_NAME} FAILED"
        else
                echo "Stop server ${SERVER_NAME} OK"
        fi
}

usage()
{
        echo "Usage: $0 [start|stop|restart]"
}

if [ $# -lt 1 ];then
        usage
        exit
fi

if [ "$1" = "start" ];then
        start

elif [ "$1" = "stop" ];then
        stop

elif [ "$1" = "restart" ];then
        stop
        start
else
        usage
fi

