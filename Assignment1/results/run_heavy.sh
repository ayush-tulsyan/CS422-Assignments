#!/bin/bash

PIN_CMD="time ../../PIN_TOOL/pin -t ../../obj-ia32/HW1.so -c 1 "

cd ./436.cactusADM
${PIN_CMD} -f 584 -o cactusADM.out -- ./cactusADM_base.i386 benchADM.par > cactusADM.ref.out 2> cactusADM.ref.err &
cd ../

cd ./437.leslie3d
${PIN_CMD} -f 2346 -o leslie3d.out -- ./leslie3d_base.i386 < leslie3d.in > leslie3d.ref.out 2> leslie3d.ref.err &
cd ../

cd ./462.libquantum
${PIN_CMD} -f 3685 -o libquantum.out -- ./libquantum_base.i386 1397 8 > libquantum.ref.out 2> libquantum.ref.err &
cd ../

cd ./470.lbm
${PIN_CMD} -f 830 -o lbm.out -- ./lbm_base.i386 3000 reference.dat 0 0 100_100_130_ldc.of > lbm.ref.out 2> lbm.ref.err &
cd ../

cd ./482.sphinx3/1/2/482.sphinx3/
${PIN_CMD} -f 1513 -o sphinx3.out -- ./sphinx3_base.i386 ctlfile . args.an4 > sphinx3.ref.out 2> sphinx3.ref.err &
cd ../

wait
