#!/bin/sh

screen -dm -S cpuvisor-service bash -l -c "./cpuvisor_service_forever.sh $*"
