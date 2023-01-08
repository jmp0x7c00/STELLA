# STELLA
Mind Your Enclave Pointers! Detecting Privacy Leakage for SGX Applications via Sparse Taint Analysis
<hr>

Our paper is the first to reveal a pointer misuse problem in SGX software that leads to privacy leakage, and we summarize five pointer-misuse patterns:
* write private data to OCALL out pointers
* write private data to OCALL in pointers
* write private data to ECALL user_check pointers
* write private data to OCALL return pointers
* write private data to Null pointers

We have implemented a prototype to detect enclave leakage bugs related to these patterns.
## Architecture
![image](https://user-images.githubusercontent.com/16433413/201198036-b17c25f0-9a14-42f0-8150-123a13e04dbc.png)

## Requirements
Our prototype is built for following system:
* Ubuntu 20.04 LTS
* Intel i7-9700T 4.30GHz 8-core CPU and 32G memory
* Intel SGX SDK 2.15
* LLVM/Clang v13
* [SVF](https://github.com/SVF-tools/SVF)

*Note: A real SGX-enable CPU is optional.*

## Source Code
```
STELLA
|
|-- src
      |-- core
             |-- ELA.cpp
      |
      |-- edl_extractor
                      |- Extractor.ml
                      |- CodeGen.ml
      |
      |-- llvm_src
      |
      |-- svf_src
|
|-- PoCs
       |-- ECALL_out_leak
       |
       |-- ECALL_user_check_leak
       |
       |-- Null_pointer_leak
       |
       |-- OCALL_in_leak
       |
       |-- OCALL_return_leak
|
|-- scripts
          |-- PoCs
          |
          |-- real-world enclaves

```

## Build
```
export PROJECT_ROOT=path_to_repository_head
```
### Install Intel SGX SDK
[Installation guide](https://github.com/intel/linux-sgx)

### Build LLVM Toolchain
1. Install required libraries
```text
sudo apt-get install linux-headers-$(uname -r) csh gawk automake libtool bison flex libncurses5-dev
# Check 'makeinfo -v'. If 'makeinfo' does not exist
sudo apt-get install apt-file texinfo texi2html
sudo apt-file update
sudo apt-file search makeinfo
```

2. Download binutils source code
```text
cd ~
git clone --depth 1 git://sourceware.org/git/binutils-gdb.git binutils
```

3. Build binutils
```text
mkdir build
cd build
../binutils/configure --enable-gold --enable-plugins --disable-werror
make
```

4. Build LLVM
```text
cd $PROJECT_ROOT/src
wget https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-13.0.0.zip
unzip llvmorg-13.0.0.zip
mv llvmorg-13.0.0 llvm_src
cd llvm_src
mkdir build
cd build
# replace path_to_binutils to the actual path.
cmake -DLLVM_ENABLE_PROJECTS=clang -DLLVM_BINUTILS_INCDIR="path_to_binutils/include" -G "Unix Makefiles" ../llvm
make -j8
```

5. Backup ar, nm, ld and ranlib:
```text
cd ~
mkdir backup
cd /usr/bin/
cp ar ~/backup/
cp nm ~/backup/
cp ld ~/backup/
cp ranlib ~/backup/
```

6. Replace ar, nm, ld and ranlib
```text
cd /usr/bin/
sudo cp ~/build/binutils/ar ./
sudo rm nm
sudo cp ~/build/binutils/nm-new ./nm
sudo cp ~/build/binutils/ranlib ./
sudo cp ~/build/gold/ld-new ./ld
```

7. Install gold plugin
```text
cd /usr/lib
sudo mkdir bfd-plugins
cd bfd-plugins
sudo cp $PROJECT_ROOT/src/llvm_src/build/lib/LLVMgold.so ./
sudo cp $PROJECT_ROOT/src/llvm_src/build/lib/libLTO.* ./
```

### Build SVF
```text
sudo apt install cmake gcc g++ libtinfo-dev libz-dev zip wget ##(If running on Ubuntu 20.04)
cd $PROJECT_ROOT/src/svf_src
source ./build.sh
```
### Build Core
```text
cd $PROJECT_ROOT/src/core_src
mkdir build
cd build
make ..
```

## Usage
Run STELLA on PoCs:
```
cd $PROJECT_ROOT/scripts/PoCs
./run.sh
```
Run STELLA on real-world enclaves:
```
cd $PROJECT_ROOT/scripts/real-world enclaves/BiORAM-SGX
./run.sh
```

## Experimental Results
* Currently, 21 bugs are confirmed and 4 bugs are fixed.

|Index|Project|Leak Type|Leak Point|Leaked Variable|Status|
|-----|-------|---------|----------|---------------|------|
|1|sgx-aes-gcm|ECALL out|[Source Location](https://github.com/rodolfoams/sgx-aes-gcm/blob/3378ba101ed9bfc555d933c669dfda5fd03235e3/CryptoEnclave/CryptoEnclave.cpp#L24)|[Source Location](https://github.com/rodolfoams/sgx-aes-gcm/blob/3378ba101ed9bfc555d933c669dfda5fd03235e3/CryptoEnclave/CryptoEnclave.cpp#L14)|[Reported](https://github.com/rodolfoams/sgx-aes-gcm/issues/2)|
|2|sgx-aes-gcm|OCALL in|[Source Location](https://github.com/rodolfoams/sgx-aes-gcm/blob/3378ba101ed9bfc555d933c669dfda5fd03235e3/CryptoEnclave/CryptoEnclave.cpp#L25)|[Source Location](https://github.com/rodolfoams/sgx-aes-gcm/blob/3378ba101ed9bfc555d933c669dfda5fd03235e3/CryptoEnclave/CryptoEnclave.cpp#L14)|[Reported](https://github.com/rodolfoams/sgx-aes-gcm/issues/2)|
|3|sgx-based-mix-networks|ECALL out       |[Source Location](https://github.com/oEscal/sgx-based-mix-networks/blob/2827f1004005ab6dca1cd060529bbae057b8cc61/mix_solution/Enclave/Enclave.cpp#L164)      |[Source Location](https://github.com/oEscal/sgx-based-mix-networks/blob/2827f1004005ab6dca1cd060529bbae057b8cc61/mix_solution/Enclave/Enclave.cpp#L156)|[Reported](https://github.com/oEscal/sgx-based-mix-networks/issues/1)|
|4|sgx-based-mix-networks|NPD             |[Source Location](https://github.com/oEscal/sgx-based-mix-networks/blob/2827f1004005ab6dca1cd060529bbae057b8cc61/mix_solution/Enclave/Enclave.cpp#L157)      |[Source Location](https://github.com/oEscal/sgx-based-mix-networks/blob/2827f1004005ab6dca1cd060529bbae057b8cc61/mix_solution/Enclave/Enclave.cpp#L155)|[Reported](https://github.com/oEscal/sgx-based-mix-networks/issues/2)|
|5|sgx_wechat_app        |OCALL in        |[Source Location](https://github.com/TonyCode2012/sgx_wechat_app/blob/56a8d55a089dc63b8bd43c06171c3c11e0a11753/Server/Enclave/Enclave.cpp#L130)              |[Source Location](https://github.com/TonyCode2012/sgx_wechat_app/blob/56a8d55a089dc63b8bd43c06171c3c11e0a11753/Server/Enclave/Enclave.cpp#L121)|[Reported](https://github.com/TonyCode2012/sgx_wechat_app/issues/2)|
|6|TACIoT                |OCALL in        |[Source Location](https://github.com/GTA-UFRJ-team/TACIoT/blob/99db93101cc881b7ce03d485b86f6b7da1ecea5d/server/server_enclave/server_enclave.cpp#L153)       |[Source Location](https://github.com/GTA-UFRJ/TACIoT/blob/99db93101cc881b7ce03d485b86f6b7da1ecea5d/server/server_enclave/server_enclave.cpp#L51)|[Fixed](https://github.com/GTA-UFRJ/TACIoT/issues/1)|
|7|password-manager      |OCALL in        |[Source Location](https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L278) |[Source Location](https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L276)|[Reported](https://github.com/ShivKushwah/password-manager/issues/3)|
|8|password-manager      |NPD             |[Source Location](https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L253)|[Source Location](https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L250)|[Reported](https://github.com/ShivKushwah/password-manager/issues/4)|
|9|BiORAM-SGX            |OCALL in        |[Source Location](https://github.com/cBioLab/BiORAM-SGX/blob/d86dab22dba12896e9e0c7ebd968ff064dcefe6b/dataowner_data/EncryptAES_SGX/Enclave/Enclave.cpp#L154)|[Source Location](https://github.com/cBioLab/BiORAM-SGX/blob/d86dab22dba12896e9e0c7ebd968ff064dcefe6b/dataowner_data/EncryptAES_SGX/Enclave/Enclave.cpp#L122)|[Reported](https://github.com/cBioLab/BiORAM-SGX/issues/3)|
|10|BiORAM-SGX            |OCALL in        |[Source Location](https://github.com/cBioLab/BiORAM-SGX/blob/d86dab22dba12896e9e0c7ebd968ff064dcefe6b/dataowner_data/EncryptAES_SGX/Enclave/Enclave.cpp#L188)|[Source Location](https://github.com/cBioLab/BiORAM-SGX/blob/d86dab22dba12896e9e0c7ebd968ff064dcefe6b/dataowner_data/EncryptAES_SGX/Enclave/Enclave.cpp#L123)|[Reported](https://github.com/cBioLab/BiORAM-SGX/issues/4)|
|11|Town-Crier            |OCALL in        |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/hybrid_cipher.cpp#L99)  |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/hybrid_cipher.cpp#L97)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/69)|
|12|Town-Crier            |OCALL in        |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/SSLClient.c#L1030)                        |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/SSLClient.c#L319)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/69)|
|13|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_bloomberg.cpp#L18)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_bloomberg.cpp#L9)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)|
|14|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/test/regex_test.cpp#L81)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/test/regex_test.cpp#L68)|[Reported](https://github.com/bl4ck5un/Town-Crier/issues/72)|
|15|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_bloomberg.cpp#L54)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_bloomberg.cpp#L28)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)
|16|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Steam2.cpp#L50)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Steam2.cpp#L33)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)
|17|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Steam2.cpp#L116)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Steam2.cpp#L94)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)
|18|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Steam2.cpp#L70)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Steam2.cpp#L57)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)
|19|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_Yahoo.cpp#L21)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_Yahoo.cpp#L11)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)|
|20|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_Google.cpp#L61)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Current_Google.cpp#L48)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)|
|21|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Transaction.cpp#L206)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Transaction.cpp#L168)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)|
|22|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Flight.cpp#L65)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/Flight.cpp#L53)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)|
|23|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/ECDAS.c#L103)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/ECDAS.c#L89)|[Confirmed](https://github.com/bl4ck5un/Town-Crier/issues/70)
|24|Town-Crier            |NPD             |[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/scrapers/steam2.cpp#L224)|[Source Location](https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/scrapers/steam2.cpp#L201)|[Reported](https://github.com/bl4ck5un/Town-Crier/issues/72)
|25|Fidelius              |OCALL in        |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1036)|[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1034)|[Reported](https://github.com/SabaEskandarian/Fidelius/issues/12)|
|26|Fidelius              |NPD             |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L999)   |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L653)|[Reported](https://github.com/SabaEskandarian/Fidelius/issues/15)|
|27|Fidelius              |NPD             |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1035)   |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1032)|[Reported](https://github.com/SabaEskandarian/Fidelius/issues/14)
|28|Fidelius              |NPD             |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_app/sgx_display/hdmichannel/rgbencoder.c#L18)  |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_app/sgx_display/hdmichannel/xoverlay.cpp#L29)|[Reported](https://github.com/SabaEskandarian/Fidelius/issues/15)
|29|Fidelius              |NPD             |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1042)  |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L653)|[Reported](https://github.com/SabaEskandarian/Fidelius/issues/15)
|30|Fidelius              |NPD             |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_app/sgx_display/btchannel.cpp#L79) |[Source Location](https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_app/sgx_display/btchannel.cpp#L77)|[Reported](https://github.com/SabaEskandarian/Fidelius/issues/15)
|31|sgx-dnet              |OCALL in        |[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/parser.c#L1194)|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/train/trainer.c#L44)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/4)
|32|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/rnn_layer.c#L53   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|33|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/rnn_layer.c#L39   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|34|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L87  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|35|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L80  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|36|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L59  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|37|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/crnn_layer.c#L47  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|38|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/crnn_layer.c#L54  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|39|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L45  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|40|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/option_list.c#L47 )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|41|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L36   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|42|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L66  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|43|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/crnn_layer.c#L61  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|44|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L43   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|45|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L38  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|46|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L50   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|47|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L73  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|48|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L57   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|49|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L52  )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|50|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L64   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|51|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/rnn_layer.c#L46   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|52|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L71   )|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Include/dnet_types.h#L148)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/5)|
|53|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/connected_layer.c#L46)|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/connected_layer.c#L35)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/6)
|54|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/deconvolutional_layer.c#L60)|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/deconvolutional_layer.c#L52)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/6)
|55|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/convolutional_layer.c#L129)|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/convolutional_layer.c#L111)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/6)
|56|sgx-dnet|NPD|[Source Location](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/local_layer.c#L59)|[Source Locatoin](https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/local_layer.c#L51)|[Reported](https://github.com/anonymous-xh/sgx-dnet/issues/6)                                                                                                                             |
|57|PrivacyGuard|OCALL in|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/DataBroker/Enclave/enclave.cpp#L490      )|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/DataBroker/Enclave/enclave.cpp#L92)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/3)
|58|PrivacyGuard|OCALL in|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/DataBroker/Enclave/enclave.cpp#L502      )|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/DataBroker/Enclave/enclave.cpp#L95)|[Fixed](https://github.com/yang-sec/PrivacyGuard/pull/4/commits)|
|59|PrivacyGuard|OCALL in|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L483 )|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L93)|[Fixed](https://github.com/yang-sec/PrivacyGuard/pull/4/commits)|
|60|PrivacyGuard|OCALL in|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L510 )|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L94)|[Fixed](https://github.com/yang-sec/PrivacyGuard/pull/4/commits)
|61|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/DataBroker/Enclave/enclave.cpp#L466)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/DataBroker/Enclave/enclave.cpp#L298)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|62|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/DataBroker/Enclave/enclave.cpp#L498)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/DataBroker/Enclave/enclave.cpp#L299)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|63|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/DataBroker/Enclave/enclave.cpp#L304)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/DataBroker/Enclave/enclave.cpp#L300)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|64|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/CEE/isv_enclave/isv_enclave.cpp#L404)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/1ef665fca9dadf00bc0bb363842ab471a747ab0a/CEE/isv_enclave/isv_enclave.cpp#L404)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|65|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L2984)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L2970)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|66|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/Enclave_testML/isv_enclave/enclave_svm.cpp#L2136)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/Enclave_testML/isv_enclave/enclave_svm.cpp#L2116)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|67|PrivacyGuard|NPD |[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE/isv_enclave/enclave_fann.cpp#L2730)|[Source Location](https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE/isv_enclave/enclave_fann.cpp#L2718)|[Confirmed](https://github.com/yang-sec/PrivacyGuard/issues/5)
|68|SGX_SQLite  |OCALL in        |line 30540 in sqlit.c|line 30539 in sqlit.c|[Reported](https://github.com/yerzhan7/SGX_SQLite/issues/8)|
|69|TaLoS       |ECALL user_check|[Source Location](https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/talos/patch/ssl_lib.c.patch#L1396) |[Source Location](https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/talos/patch/ssl_lib.c.patch#L1395)|[Confirmed](https://github.com/lsds/TaLoS/issues/33)|
|70|TaLoS       |ECALL user_check|[Source Location](https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/libressl-2.4.1/ssl/ssl_rsa.c#L209) |[Source Location](https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/libressl-2.4.1/ssl/ssl_rsa.c#L150)|[Reported]()
|71|TaLoS       |ECALL out       |[Source Location](https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/talos/patch/ssl_lib.c.patch#L1190)|[Source Location](https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/talos/patch/ssl_lib.c.patch#L1190)|[Reported](https://github.com/lsds/TaLoS/issues/35)|
|72|SGX-Tor     |OCALL return    |[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/rendservice.c#L1254)  |[Source Location]() | [Reported]()
|73|SGX-Tor|OCALL in|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/control.c#L3883)|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/control.c#L3851)|[Reported](https://github.com/kaist-ina/SGX-Tor/issues/5)|
|74|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L158)|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L157) |[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
|75|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L141) |[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L141)|[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
|76|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/SGX-Tor_WIN/TorVS2012/TorSGX/TorSGX.cpp#L138) |[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/SGX-Tor_WIN/TorVS2012/TorSGX/TorSGX.cpp#L138)|[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
|77|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L89)|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L90 ) |[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
|78|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L707) |[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L706)|[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
|79|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L473) |[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/SGX-Tor_WIN/OpenSSL_APP/include/openssl/buffer.h#L79)|[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
|80|SGX-Tor|NPD|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L191)|[Source Location](https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L192) |[Reported](https://github.com/kaist-ina/SGX-Tor/issues/7)
   
## Screenshots
