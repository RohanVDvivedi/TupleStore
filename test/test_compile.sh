cd ..
make clean all
cd test
gcc ./test.c -o test.out -I${SIMPLE_TUPLE_STORAGE_MODEL_PATH}/inc -L${SIMPLE_TUPLE_STORAGE_MODEL_PATH}/bin -lstupstom