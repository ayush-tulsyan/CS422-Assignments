PREDICTOR=$1
if [ -z $PREDICTOR ]; then
    echo "Usage: ./run_tests <PREDICTOR>"
    exit 1
fi
echo "Running the tests for predictor=$PREDICTOR"

if type "parallel" > /dev/null; then
    parallel -k echo {2} ';' ./$PREDICTOR {1}/{2} \
        ::: ./traces/without-values/ \
        ::: $(ls ./traces/without-values/ | grep "\." | grep -o "^[^.]*")
else
    output_DIST_FP_1=$(./$PREDICTOR traces/without-values/DIST-FP-1 &)
    output_DIST_FP_2=$(./$PREDICTOR traces/without-values/DIST-FP-2 &)
    output_DIST_FP_3=$(./$PREDICTOR traces/without-values/DIST-FP-3 &)
    output_DIST_FP_4=$(./$PREDICTOR traces/without-values/DIST-FP-4 &)
    output_DIST_FP_5=$(./$PREDICTOR traces/without-values/DIST-FP-5 &)
    output_DIST_INT_1=$(./$PREDICTOR traces/without-values/DIST-INT-1 &)
    output_DIST_INT_2=$(./$PREDICTOR traces/without-values/DIST-INT-2 &)
    output_DIST_INT_3=$(./$PREDICTOR traces/without-values/DIST-INT-3 &)
    output_DIST_INT_4=$(./$PREDICTOR traces/without-values/DIST-INT-4 &)
    output_DIST_INT_5=$(./$PREDICTOR traces/without-values/DIST-INT-5 &)
    output_DIST_MM_1=$(./$PREDICTOR traces/without-values/DIST-MM-1 &)
    output_DIST_MM_2=$(./$PREDICTOR traces/without-values/DIST-MM-2 &)
    output_DIST_MM_3=$(./$PREDICTOR traces/without-values/DIST-MM-3 &)
    output_DIST_MM_4=$(./$PREDICTOR traces/without-values/DIST-MM-4 &)
    output_DIST_MM_5=$(./$PREDICTOR traces/without-values/DIST-MM-5 &)
    output_DIST_SERV_1=$(./$PREDICTOR traces/without-values/DIST-SERV-1 &)
    output_DIST_SERV_2=$(./$PREDICTOR traces/without-values/DIST-SERV-2 &)
    output_DIST_SERV_3=$(./$PREDICTOR traces/without-values/DIST-SERV-3 &)
    output_DIST_SERV_4=$(./$PREDICTOR traces/without-values/DIST-SERV-4 &)
    output_DIST_SERV_5=$(./$PREDICTOR traces/without-values/DIST-SERV-5 &)
    wait

    echo ""
    echo "Printing the results"
    echo "DIST_FP_1"
    echo "$output_DIST_FP_1"

    echo "DIST_FP_2"
    echo "$output_DIST_FP_2"

    echo "DIST_FP_3"
    echo "$output_DIST_FP_3"

    echo "DIST_FP_4"
    echo "$output_DIST_FP_4"

    echo "DIST_FP_5"
    echo "$output_DIST_FP_5"

    echo "DIST_INT_1"
    echo "$output_DIST_INT_1"

    echo "DIST_INT_2"
    echo "$output_DIST_INT_2"

    echo "DIST_INT_3"
    echo "$output_DIST_INT_3"

    echo "DIST_INT_4"
    echo "$output_DIST_INT_4"

    echo "DIST_INT_5"
    echo "$output_DIST_INT_5"

    echo "DIST_MM_1"
    echo "$output_DIST_MM_1"

    echo "DIST_MM_2"
    echo "$output_DIST_MM_2"

    echo "DIST_MM_3"
    echo "$output_DIST_MM_3"

    echo "DIST_MM_4"
    echo "$output_DIST_MM_4"

    echo "DIST_MM_5"
    echo "$output_DIST_MM_5"

    echo "DIST_SERV_1"
    echo "$output_DIST_SERV_1"

    echo "DIST_SERV_2"
    echo "$output_DIST_SERV_2"

    echo "DIST_SERV_3"
    echo "$output_DIST_SERV_3"

    echo "DIST_SERV_4"
    echo "$output_DIST_SERV_4"

    echo "DIST_SERV_5"
    echo "$output_DIST_SERV_5"
fi
