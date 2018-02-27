PREDICTOR=$1
PREDICTOR_BINARY=$(echo $PREDICTOR | grep -o "^[^.]*")

if [ -z $PREDICTOR ]; then
    echo "Usage: ./compile_predictor.sh <PREDICTOR>"
    exit 1
fi

echo $PREDICTOR
echo $PREDICTOR_BINARY

cp ./predictor.h ./__original_predictor.h.bck
cp $PREDICTOR ./predictor.h
make
mv ./predictor $PREDICTOR_BINARY
mv ./__original_predictor.h.bck ./predictor.h
