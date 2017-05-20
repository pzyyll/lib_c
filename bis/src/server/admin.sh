#!/bin/sh

ulimit -c unlimited 

SERVER_NAME="server"
SERVER_BIN_PATH="./"
SERVER_PARAM="./${SERVER_NAME}.ini"
SEVER_TIMER_MS=10000	#10s

start()
{
        ${SERVER_BIN_PATH}/${SERVER_NAME} --conf_file=${SERVER_PARAM} --tick_timer=${SEVER_TIMER_MS} -D start 
        if [ $? -eq 0 ];then
                echo "Start server ${SERVER_NAME} OK"
        else
                echo "Start server ${SERVER_NAME} FAILED"
        fi
}

stop()
{
	${SERVER_BIN_PATH}/${SERVER_NAME} stop
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


clean_db()
{
        echo "mv ../bs.db bs.db.`date +%Y%m%d%H%M%S`"
        mv bs.db bs.db.`date +%Y%m%d%H%M%S`
        exit 0
}

usage()
{
        echo "Usage: $0 [start|stop|restart|clean_db]"
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
elif [ "$1" = "clean_db" ];then
        stop
	clean_db
else
        usage
fi

