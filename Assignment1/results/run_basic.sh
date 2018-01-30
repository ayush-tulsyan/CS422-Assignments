#!/bin/bash

PIN_CMD="time ../../PIN_TOOL/pin -t ../../obj-ia32/HW1.so -c 1 "

cd ./400.perlbench
${PIN_CMD} -f 207 -o perlbench.diffmail.out -- ./perlbench_base.i386 -I./lib diffmail.pl 4 800 10 17 19 300 > perlbench.ref.diffmail.out 2> perlbench.ref.diffmail.err &
cd ..

cd ./401.bzip2
${PIN_CMD} -f 301 -o bzip2.source.out -- ./bzip2_base.i386 input.source 280 > bzip2.ref.source.out 2> bzip2.ref.source.err &
cd ..

cd ./403.gcc
${PIN_CMD} -f 107 -o gcc.cp-decl.out -- ./gcc_base.i386 cp-decl.i -o cp-decl.s > gcc.ref.cp-decl.out 2> gcc.ref.cp-decl.err &
cd ..

cd ./429.mcf
${PIN_CMD} -f 377 -o mcf.out -- ./mcf_base.i386 inp.in > mcf.ref.out 2> mcf.ref.err &
cd ..

cd ./450.soplex
${PIN_CMD} -f 364 -o soplex.ref.out -- ./soplex_base.i386 -m3500 ref.mps > soplex.ref.ref.out 2> soplex.ref.ref.err &
cd ..

cd ./456.hmmer
${PIN_CMD} -f 264 -o hmmr.nph3.out -- ./hmmer_base.i386 nph3.hmm swiss41 > hmmer.ref.nph3.out 2> hmmer.ref.nph3.err &
cd ..

cd ./471.omnetpp
${PIN_CMD} -f 43  -o omnetpp.out -- ./omnetpp_base.i386 omnetpp.ini > omnetpp.ref.log 2> omnetpp.ref.err &
cd ..

cd ./483.xalancbmk
${PIN_CMD} -f 1331 -o xalancbmk.out -- ./xalancbmk_base.i386 -v t5.xml xalanc.xsl > xalancbmk.ref.out 2> xalancbmk.ref.err &
cd ..

wait
