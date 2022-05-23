# include "headers/test.h"
# include "headers/ela.h"

#define BEGIN(func) cout << "" #func "" \
                         << " start..." << endl
#define END(func) cout << "" #func "" \
                       << " end..." << endl


void test_ecall_out_ptr()
{
    std::set<std::string> expected{
        "{ ln: 10  cl: 11  fl: Enclave/Enclave.cpp }", 
        "{ ln: 14  cl: 6  fl: Enclave/Enclave.cpp }"
    };
    BEGIN(test_ecall_out_ptr);
    std::vector<std::string> mnv = {"/home/yang/ELA/testcase/test1_ecall_out_pointer_leak.ll"};
    ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/test.config");
    // ela->printICFGTraverseTime();
    clock_t start = clock();
    ela->findSinks();
    set<string> locs;
    ela->getSinksSourceLocations(locs);
    assert(locs == expected);
    // ela->backwardTracking();
    printUseTime("findSink time", start);
    delete ela;
    END(test_ecall_out_ptr);
}

void test_user_check_ptr()
{
    std::set<std::string> expected{
        "{ ln: 11  cl: 10  fl: Enclave/Enclave.cpp }"
    };
    BEGIN(test_user_check_ptr);
    std::vector<std::string> mnv = {"/home/yang/ELA/testcase/test2_user_check_pointer_leak.ll"};
    ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/test.config");
    // ela->printICFGTraverseTime();
    clock_t start = clock();
    ela->findSinks();
    set<string> locs;
    ela->getSinksSourceLocations(locs);
    assert(locs == expected);
    // ela->backwardTracking();
    printUseTime("findSink time", start);
    delete ela;
    END(test_user_check_ptr);
}

void test_ocall_return_ptr()
{
    std::set<std::string> expected{
        "{ ln: 16  cl: 7  fl: Enclave/Enclave.cpp }"
    };
    BEGIN(test_ocall_return_ptr);
    cout << "test_ocall_return_ptr start..." << endl;
    std::vector<std::string> mnv = {"/home/yang/ELA/testcase/test3_return_pointer_leak.ll"};
    ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/test.config");
    clock_t start = clock();
    ela->findSinks();
    set<string> locs;
    ela->getSinksSourceLocations(locs);
    assert(locs == expected);
    // ela->backwardTracking();
    printUseTime("findSink time", start);
    delete ela;
    END(test_ocall_return_ptr);
}
void test_ocall_in_ptr()
{
    std::set<std::string> expected{
        "{ ln: 6  cl: 5  fl: Enclave/Enclave.cpp }"
    };
    BEGIN(test_ocall_in_ptr);
    std::vector<std::string> mnv = {"/home/yang/ELA/testcase/test4_ocall_in_pointer_leak.ll"};
    ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/test.config");
    clock_t start = clock();
    ela->findSinks();
    set<string> locs;
    ela->getSinksSourceLocations(locs);
    assert(locs == expected);
    // ela->backwardTracking();
    printUseTime("findSink time", start);
    delete ela;
    END(test_ocall_in_ptr);
}

void test_indirect_call()
{
    std::set<std::string> expected{
        "{ ln: 10  cl: 9  fl: indirectCall.c }"
    };
    BEGIN(test_indirect_call);
    std::vector<std::string> mnv = {"/home/yang/ELA/testcase/indirectCall.ll"};
    ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/test.config");
    clock_t start = clock();
    ela->findSinks();
    set<string> locs;
    ela->getSinksSourceLocations(locs);
    assert(locs == expected);
    // ela->backwardTracking();
    printUseTime("findSink time", start);
    delete ela;
    END(test_ocall_in_ptr);
}

void regressionTest()
{
    cout << "================================regressionTest BEGIN================================" << endl;
    test_ecall_out_ptr();
    test_user_check_ptr();
    test_ocall_return_ptr();
    test_ocall_in_ptr();
    test_indirect_call();
    cout << "================================regressionTest END================================" << endl;
}