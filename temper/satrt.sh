
./exam-fmc146v-sync --exam-ini ../setup_files/exam_fmc146v_sync.ini_internal_clk_start
./isvi_service.sh -p 5555 -m start
./exam-adc ../setup_files/exam_adc.ini3gda -b 1


#./exam-adc ../setup_files/exam_edac.ini_3gda_r_101 -b 1