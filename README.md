# Mind Your Enclave Pointers! Detecting Privacy Leakage for SGX Applications via Sparse Taint Analysis
Our paper is the first to reveal a pointer misuse problem in SGX software that leads to privacy leakage, and we summarize five pointer-misuse patterns:
* write private data to OCALL out pointer
* write private data to OCALL in pointer
* write private data to ECALL user_check pointer
* write private data to OCALL return pointer
* write private data to Null pointer

We have implemented a prototype to detect enclave leakage bugs related to these patterns.
## Prototype
Our prototype is built for following system:
* Ubuntu 20.04 LTS
* Intel i7-9700T 4.30GHz 8-core CPU and 32G memory
* Intel SGX SDK 2.15
* LLVM/Clang v13
* SVF

*Note: A real SGX-enable CPU is optional.*

## Source Code
## Build
### Intel SGX SDK
### LLVM Tool
### SVF

## Usage

## Screenshots

## Experimental results
