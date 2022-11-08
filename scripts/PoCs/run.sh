export PROJECT_ROOT="$PWD""/../../"
export LLVM_BUILD="$PROJECT_ROOT/src/semantics/llvm_src/build"
export SEMANTICS_DIR="$PROJECT_ROOT/src/semantics/pyedl"
export SYMEMU="$PROJECT_ROOT/src/core/Triton/src/enclaveCoverage"

source /opt/intel/sgxsdk/environment

# micro-benchmark
EO_SRC="$PROJECT_ROOT/PoCs/ECALL_out_leak"
EU_SRC="$PROJECT_ROOT/PoCs/ECALL_user_check_leak"
OI_SRC="$PROJECT_ROOT/PoCs/OCALL_in_leak"
OR_SRC="$PROJECT_ROOT/PoCs/OCALL_return_leak"
NP_SRC="$PROJECT_ROOT/PoCs/Null_pointer_leak"

printf "Select your benchmark:\n1)ECALL out leak\n2)ECALL user_check leak\n3)OCALL in leak\n4)OCALL return leak\n5)Null pointer leak\n"
read choice

if [ $choice -eq 1 ]
then
	PROJECT_DIR=$EO_SRC
elif [ $choice -eq 2 ]
then
	PROJECT_DIR=$EU_SRC
elif [ $choice -eq 3 ]
then
	PROJECT_DIR=$OI_SRC
elif [ $choice -eq 4 ]
then
	PROJECT_DIR=$OR_SRC
elif [ $choice -eq 5 ]
then
	PROJECT_DIR=$NP_SRC
else
	echo "Wrong choice."
	exit 1
fi

cd $PROJECT_DIR
make clean
make SGX_MODE=SIM
cd "$OLDPWD"