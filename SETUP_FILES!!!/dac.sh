#!/bin/bash

./isvi_service.sh -p 5555 -m start
./exam-fmc146v-sync --exam-ini ini/exam_fmc146v_sync.ini
./exam-edac ini/exam_edac.ini_3gda_r_101 -b 1