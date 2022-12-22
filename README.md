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
|Index|Project               |Leak Type       |Vulnerable  Code Location                                                                                                                 |
|--|----------------------|----------------|------------------------------------------------------------------------------------------------------------------------------------------|
|1|sgx-aes-gcm           |ECALL out       |https://github.com/rodolfoams/sgx-aes-gcm/blob/3378ba101ed9bfc555d933c669dfda5fd03235e3/CryptoEnclave/CryptoEnclave.cpp#L24               |
|2|sgx-aes-gcm           |OCALL in        |https://github.com/rodolfoams/sgx-aes-gcm/blob/3378ba101ed9bfc555d933c669dfda5fd03235e3/CryptoEnclave/CryptoEnclave.cpp#L25               |
|3|sgx-based-mix-networks|ECALL out       |https://github.com/oEscal/sgx-based-mix-networks/blob/2827f1004005ab6dca1cd060529bbae057b8cc61/mix_solution/Enclave/Enclave.cpp#L164      |
|4|sgx-based-mix-networks|NPD             |https://github.com/oEscal/sgx-based-mix-networks/blob/2827f1004005ab6dca1cd060529bbae057b8cc61/mix_solution/Enclave/Enclave.cpp#L156      |
|5|sgx_wechat_app        |OCALL in        |https://github.com/TonyCode2012/sgx_wechat_app/blob/56a8d55a089dc63b8bd43c06171c3c11e0a11753/Server/Enclave/Enclave.cpp#L130              |
|6|sgx_wechat_app        |OCALL in        |https://github.com/TonyCode2012/sgx_wechat_app/blob/56a8d55a089dc63b8bd43c06171c3c11e0a11753/Server/Enclave/Enclave.cpp#L149              |
|7|TACIoT                |OCALL in        |https://github.com/GTA-UFRJ-team/TACIoT/blob/99db93101cc881b7ce03d485b86f6b7da1ecea5d/server/server_enclave/server_enclave.cpp#L153       |
|8|TACIoT                |OCALL in        |https://github.com/GTA-UFRJ-team/TACIoT/blob/99db93101cc881b7ce03d485b86f6b7da1ecea5d/server/server_enclave/server_enclave.cpp#L163       |
|9|password-manager      |OCALL in        |https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L278                    |
|10|password-manager      |ECALL out       |https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L188                    |
|11|password-manager      |NPD             |https://github.com/ShivKushwah/password-manager/blob/100cdcdbc14b49a3118f6cbca445eddfa6009e41/Enclave/Enclave.cpp#L23
|12|BiORAM-SGX            |OCALL in        |https://github.com/cBioLab/BiORAM-SGX/blob/d86dab22dba12896e9e0c7ebd968ff064dcefe6b/dataowner_data/EncryptAES_SGX/Enclave/Enclave.cpp#L154|
|13|Town-Crier            |OCALL in        |https://github.com/bl4ck5un/Town-Crier/blob/78e19969dddf0964da9db1e9d1043e62f231daea/src/Enclave/hybrid_cipher.cpp#L99                    |
|14|Town-Crier            |OCALL in        |https://github.com/bl4ck5un/Town-Crier/blob/33471ff56cb75c9672a51c9d9c20352c96cc3444/win/Enclave/SSLClient.c#L1030                        |
|15-26|Town-Crier            |NPD             |https://github.com/bl4ck5un/Town-Crier/issues/70                                                                                                                                |
|27|Fidelius              |OCALL in        |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1036   |
|28|Fidelius              |OCALL in        |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1020   |
|29|Fidelius              |NPD             |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L996   |
|30|Fidelius              |NPD             |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1034   |
|31|Fidelius              |NPD             |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1038  |
|32|Fidelius              |NPD             |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L1040  |
|33|Fidelius              |NPD             |https://github.com/SabaEskandarian/Fidelius/blob/ab0d846506d2545ce570f295e154481c75a73a47/web_enclave/isv_enclave/isv_enclave.cpp#L984  |
|34|sgx-dnet              |OCALL in        |https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/parser.c#L1104                 |
|35|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/rnn_layer.c#L53   |
|36|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/rnn_layer.c#L39   |
|37|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L87  |
|38|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L80  |
|39|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L59  |
|40|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/crnn_layer.c#L47  |
|41|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/crnn_layer.c#L54  |
|42|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L45  |
|43|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/option_list.c#L47 |
|44|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L36   |
|45|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L66  |
|46|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/crnn_layer.c#L61  |
|47|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L43   |
|48|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L38  |
|49|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L50   |
|50|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L73  |
|51|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L57   |
|52|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/lstm_layer.c#L52  |
|53|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L64   |
|54|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/rnn_layer.c#L46   |
|55|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/gru_layer.c#L71   |
|56|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/list.c#L8         |
|57|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/matrix.c#L51      |
|58|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/parser.c#L657     |
|59|sgx-dnet|NPD|https://github.com/anonymous-xh/sgx-dnet/blob/0fe09ccb9aa622d55b1b78ffd552feabe34f34e3/Enclave/dnet-in/src/data.c#L1709      |                                                                                                                             |
|60|PrivacyGuard          |OCALL in        |https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/DataBroker/Enclave/enclave.cpp#L490                |
|61|PrivacyGuard          |OCALL in        |https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/DataBroker/Enclave/enclave.cpp#L502                |
|62|PrivacyGuard          |OCALL in        |https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L483           |
|63|PrivacyGuard          |OCALL in        |https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/CEE_old/isv_enclave/isv_enclave.cpp#L510           |
|64|PrivacyGuard          |OCALL in        |https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/iDataAgent/Enclave/enclave.cpp#L461                |
|65|PrivacyGuard          |OCALL in        |https://github.com/yang-sec/PrivacyGuard/blob/94e888aaaf3db019d61a6585aaecf6780bccb408/iDataAgent/Enclave/enclave.cpp#L473                |
|66-72|PrivacyGuard       |NPD             |https://github.com/yang-sec/PrivacyGuard/issues/5                        |
|73|SGX_SQLite            |OCALL in        |https://github.com/yerzhan7/SGX_SQLite/issues/8                                                                                           |
|74|TaLoS                 |ECALL user_check|https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/talos/patch/ssl_lib.c.patch#L1396                         |
|75|TaLoS                 |ECALL out       |https://github.com/lsds/TaLoS/blob/052a93d6f62720a9027a56274e060b9bc84ea978/src/talos/patch/ssl_lib.c.patch#L1190                         |
|76|SGX-Tor               |OCALL return    |https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/rendservice.c#L1254                     |
|77|SGX-Tor|NPD| https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L158|
|78|SGX-Tor|NPD|https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L139 |
|79|SGX-Tor|NPD|https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L138 |
|80|SGX-Tor|NPD|https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L90  |
|81|SGX-Tor|NPD|https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L474 |
|82|SGX-Tor|NPD|https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L473 |
|83|SGX-Tor|NPD|https://github.com/kaist-ina/SGX-Tor/blob/193d4f072d49799a25830c75ef7b29f0f960e66d/Enclave/TorSGX/TorSGX.cpp#L192 |

   
## Screenshots
