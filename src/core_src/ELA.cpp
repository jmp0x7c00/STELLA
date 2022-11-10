#include "headers/LeakParams.h"
#include <iostream>
#include "SVF-FE/LLVMUtil.h"
#include "Graphs/SVFG.h"
#include "WPA/Andersen.h"
#include "SVF-FE/SVFIRBuilder.h"
#include "Util/Options.h"
#include <time.h>
#include "headers/ela.h"
#include "headers/test.h"

#define DEBUG 0
#define REGRESSION 0

using namespace llvm;
using namespace std;
using namespace SVF;
using namespace SVFUtil;


void ELA::DFS(set<const PTACallGraphNode *> &visited, vector<const PTACallGraphNode *> &path, const PTACallGraphNode *src, const PTACallGraphNode *dst)
{
    if (src->getId() == dst->getId())
    {
        path.push_back(src);
        for (auto i : path)
        {
            std::cout << i->getFunction()->getName() << "====>";
        }
        std::cout << std::endl;
        path.pop_back();
        return;
    }

    if (src->getOutEdges().size() == 0)
    {
        std::cout << "已到尽头!" << std::endl;
        return;
    }

    visited.insert(src);
    path.push_back(src);
    for (auto edge : src->getOutEdges())
    {
        // for loop
        if (visited.find(edge->getDstNode()) == visited.end())
        {
            DFS(visited, path, edge->getDstNode(), dst);
        }
    }
    visited.erase(src);
    path.pop_back();
}

void ELA::detectNullUseAnderson()
{
    for (auto it = icfg->begin(), iEnd = icfg->end(); it != iEnd; it++)
    {
        auto node = it->second;
        if (CallICFGNode::classof(node))
        {
            CallICFGNode *callICFGNode = (CallICFGNode *)node;
            CallInst *ci = (CallInst *)(callICFGNode->getCallSite());
            Function *callee = ci->getCalledFunction();
            string calleeFuncName = "";
            if (!callee)
            {
                for (auto indirectCall : callgraph->getIndCallMap())
                {
                    for (auto svfcallee : indirectCall.second)
                    {
                        calleeFuncName = svfcallee->getLLVMFun()->getName().str();
#if DEBUG
                        std::cout << "indirect call:" << calleeFuncName << std::endl;
#endif
                    }
                }
            }
            else
            {
                calleeFuncName = callee->getName().str();
#if DEBUG
                std::cout << "direct call:" << calleeFuncName << std::endl;
#endif
            }

            // if (calleeFuncName.compare("memcpy") == 0)
            // {
            // cout << calleeFuncName << endl;
            // }
            if (calleeFuncName.find("memcpy") != string::npos)
            {
                auto dst = callICFGNode->getActualParms()[0];
                auto dstPagId = pag->getValueNode(dst->getValue());
                if (ander->getPts(dstPagId).empty())
                {
                    cout << callICFGNode->toString() << endl;
                }
            }
        }

        // if(IntraICFGNode::classof(node)){
        //     IntraICFGNode* intraICFGNode = (IntraICFGNode*)node;
        //     auto stmts = intraICFGNode->getSVFStmts();
        //     for(auto stmt : stmts){
        //         cout << stmt->toString() << endl;
        //         if(StoreStmt::classof(stmt)){
        //             StoreStmt* storeStmt = (StoreStmt*)stmt;
        //             auto storeDstId = storeStmt->getLHSVarID();
        //             if (ander->getPts(storeDstId).empty()){
        //                 cout << storeStmt->toString() << endl;
        //             }

        //         }
        //     }
        // }
    }
}

void ELA::getSinksSourceLocations(set<string> &locs)
{

    // getSourceLoc(getInst())
    for (auto sink : sinks)
    {
        ICFGNode *node = icfg->getGNode(sink);
        if (CallICFGNode::classof(node))
        {
            CallICFGNode *callICFGNode = (CallICFGNode *)node;
            std::string loc = getSourceLoc(callICFGNode->getCallSite());
            assert(loc != "");
            locs.insert(loc);
        }
        else if (IntraICFGNode::classof(node))
        {
            IntraICFGNode *intraICFGNode = (IntraICFGNode *)node;
            intraICFGNode->toString();
            std::string loc = getSourceLoc(intraICFGNode->getInst());
            assert(loc != "");
            locs.insert(loc);
        }
        else
        {
            assert(0 == 1);
        }
    }
}
void ELA::findMallocNotCheck()
{
    // auto start = clock();
    // for (auto it = callgraph->begin(), iEnd = callgraph->end(); it != iEnd; it++)
    // {
    //     auto callGraphNode = it->second;

    //     auto func = callGraphNode->getFunction();
    //     if (!func)
    //     {
    //         return;
    //     }
    //     auto funcName = func->getName();
    //     if (funcName.find("alloc") != funcName.npos)
    //     {
    //         PTACallGraphEdge::CallInstSet csSet;
    //         callgraph->getAllCallSitesInvokingCallee(func, csSet);
    //         for (auto callICFGNode : csSet)
    //         {
    //             bool isAllocSite = false;
    //             for (auto edge : callICFGNode->getSVFStmts())
    //             {
    //                 if (AddrStmt::classof(edge))
    //                 {
    //                     isAllocSite = true;
    //                     break;
    //                 }
    //             }
    //             if (!isAllocSite)
    //             {
    //                 continue;
    //             }
    //             auto var = callICFGNode->getRetICFGNode()->getActualRet();
    //             auto pagNodeId = var->getId();
    //             auto svfgDefNode = svfg->getDefSVFGNode(var);
    //             bool hasCompared = false;
    //             bool hasStored = false;
    //             FIFOWorkList<const VFGNode *> worklist;
    //             Set<const VFGNode *> visited;
    //             worklist.push(svfgDefNode);

    //             /// Traverse along VFG
    //             while (!worklist.empty())
    //             {
    //                 const VFGNode* vNode = worklist.pop();
    //                 for (VFGNode::const_iterator it = vNode->OutEdgeBegin(), eit =
    //                             vNode->OutEdgeEnd(); it != eit; ++it)
    //                 {
    //                     VFGEdge* edge = *it;
    //                     VFGNode* succNode = edge->getDstNode();
    //                     if (visited.find(succNode) == visited.end())
    //                     {
    //                         visited.insert(succNode);
    //                         worklist.push(succNode);
    //                     }
    //                 }
    //             }

    //             /// Collect all LLVM Values
    //             for(Set<const VFGNode*>::const_iterator it = visited.begin(), eit = visited.end(); it!=eit; ++it)
    //             {
    //                 const VFGNode* item = *it;
    //                 auto tmp = item->toString();
    //                 if (CmpVFGNode::classof(item))
    //                 {
    //                     CmpVFGNode *cmpVFGNode = (CmpVFGNode *)item;
    //                     auto cmpPtr = cmpVFGNode->getOpVer(0)->getId();
    //                     if (SVF::AliasResult::MayAlias == ander->alias(cmpPtr, pagNodeId))
    //                     {
    //                         hasCompared = true;
    //                         break;
    //                     }
    //                 }else if(StoreVFGNode::classof(item) && (tmp.find("memcpy") != tmp.npos || tmp.find("strcpy") != tmp.npos))
    //                 {
    //                     auto storeVFGNode = (StoreVFGNode *)item;
    //                     if (SVF::AliasResult::MayAlias == ander->alias(storeVFGNode->getPAGDstNodeID(), pagNodeId))
    //                     {
    //                         if (!hasCompared)
    //                         {
    //                             hasStored = true;
    //                             sinks.insert(callICFGNode->getId());
    //                             std::cout << "\n**********************NULL BUFFER COPY******************" << std::endl;
    //                             std::cout << "alloc site:" << endl;
    //                             std::cout << callICFGNode->toString() << std::endl;
    //                             std::cout << std::endl
    //                                       << std::endl;
    //                             std::cout << "buffer copy:" << endl;
    //                             std::cout << storeVFGNode->toString() << std::endl;
    //                             std::cout << "**********************NULL BUFFER COPY******************" << std::endl;
    //                             std::cout << std::endl
    //                                       << std::endl;
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    clock_t start = clock();
    auto totalCGNum = callgraph->getTotalNodeNum();
    for (auto it = callgraph->begin(), iEnd = callgraph->end(); it != iEnd; it++)
    {
        std::cout << totalCGNum-- << std::endl;
        auto node = it->second;

        auto callee = node->getFunction();
        if (callee)
        {
            string calleeFuncName = callee->getName();
            if (calleeFuncName.find("alloc") != string::npos)
            {
                PTACallGraphEdge::CallInstSet csSet;
                callgraph->getAllCallSitesInvokingCallee(callee, csSet);
                for (auto cs : csSet)
                {
                    auto tmp = cs->toString();
                    if (tmp.find("nclave_t.c") != tmp.npos){
                        continue;
                    }
                    auto stmts = cs->getSVFStmts();
                    for (auto stmt : stmts)
                    {
                        if (AddrStmt::classof(stmt))
                        {
                            bool checked = false;
                            bool stored = false;
                            auto obj = stmt->getSrcNode()->getId();
                            auto aliasPointers = ander->getRevPts(obj);
                            for (auto pointer : aliasPointers)
                            {
                                if (!pag->hasGNode(pointer))
                                {
                                    continue;
                                }
                                auto inEdges = pag->getGNode(pointer)->getInEdges();
                                auto outEdges = pag->getGNode(pointer)->getOutEdges();
                                for (auto edge : outEdges)
                                {
                                    if (CmpStmt::classof(edge))
                                    {
                                        checked = true;
                                    }
                                }

                                for (auto edge : inEdges)
                                {
                                    if (StoreStmt::classof(edge))
                                    {
                                        auto s = edge->toString();
                                        // only focus on large memory copy
                                        if (s.find("memcpy") != s.npos || s.find("strcpy") != s.npos)
                                        {
                                            if (!checked)
                                            {
                                                
                                                if (cs->getFun()->getName().compare(edge->getICFGNode()->getFun()->getName()) == 0){
                                                    std::cout << "**********************NULL BUFFER COPY(intra)******************" << std::endl;
                                                }else{
                                                    std::cout << "**********************NULL BUFFER COPY(inter)******************" << std::endl;
                                                }
                                                std::cout << "alloc site:" << endl;
                                                std::cout << cs->toString() << endl;
                                                std::cout << std::endl
                                                          << std::endl;
                                                std::cout << "buffer copy:" << endl;
                                                std::cout << edge->toString() << endl;
                                                std::cout << "*********************************************************" << std::endl;
                                                std::cout << std::endl
                                                          << std::endl;
                                                sinks.insert(cs->getId());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "findMallocNotCheck  time: " << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    std::cout << "icfg total node num: " << icfg->getTotalNodeNum() << endl;
    std::cout << "cg total node num: " << callgraph->getTotalNodeNum() << endl;
}
void ELA::printSinks()
{
    std::cout << "\n======================sinks==========================" << std::endl;
    for (auto id : sinks)
    {
        std::cout << icfg->getGNode(id)->toString() << std::endl;
    }
    std::cout << "=====================================================" << std::endl;
}
void ELA::ptrTaint(const SVF::SVFGNode *vNode)
{
#if DEBUG
    std::cout << "def node:" << std::endl;
    std::cout << vNode->toString() << std::endl;
    std::cout << "taint propagating..." << std::endl;
#endif
    FIFOWorkList<const VFGNode *> worklist;
    Set<const VFGNode *> visited;
    worklist.push(vNode);
    while (!worklist.empty())
    {
        const VFGNode *vNode = worklist.pop();

        for (VFGNode::const_iterator it = vNode->OutEdgeBegin(), eit =
                                                                     vNode->OutEdgeEnd();
             it != eit; ++it)
        {

            VFGEdge *edge = *it;
            VFGNode *succNode = edge->getDstNode();
#if DEBUG
            cout << "processing: " << succNode->toString() << endl;
            cout << "taint set: ";
            for (auto id : taintedPagNodeIDs)
            {
                cout << id << ",";
            }
            cout << endl;
#endif

            //[外带的指针，传播规则]
            // load statement
            // q = *p 如果指针p是tainted，那么读出来的值也标记为tainted
            if (LoadVFGNode::classof(succNode))
            {
                LoadVFGNode *loadVFGNode = (LoadVFGNode *)succNode;
                NodeID srcNodeID = loadVFGNode->getPAGSrcNodeID();
                if (taintedPagNodeIDs.find(srcNodeID) != taintedPagNodeIDs.end())
                {
                    NodeID dstNodeID = loadVFGNode->getPAGDstNodeID();
#if DEBUG
                    cout << "add pag node: " << dstNodeID << endl;
#endif
                    taintedPagNodeIDs.insert(dstNodeID);
                }
            }
            //[外带的指针，传播规则]
            // copy statement，bitcast命令
            // q = p,如果指针p是tainted，q也标记为tainted
            else if (CopyVFGNode::classof(succNode))
            {
                CopyVFGNode *copyVFGNode = (CopyVFGNode *)succNode;
                NodeID srcNodeID = copyVFGNode->getPAGSrcNodeID();
                if (taintedPagNodeIDs.find(srcNodeID) != taintedPagNodeIDs.end())
                {
                    NodeID dstNodeID = copyVFGNode->getPAGDstNodeID();
#if DEBUG
                    cout << "add pag node: " << dstNodeID << endl;
#endif
                    taintedPagNodeIDs.insert(dstNodeID);
                }
            }
            //[外带的指针，传播规则]
            //*q = p,如果p是tainted，p存入q所指向的内存，q也应该被标记为tainted
            //       如果p是untainted，p存入q所指向的内存，指针q应该被洗白，q去除污点标记，同时，这里是一处sink点
            else if (StoreVFGNode::classof(succNode))
            {
                StoreVFGNode *storeVFGNode = (StoreVFGNode *)succNode;
                NodeID dstNodeID = storeVFGNode->getPAGDstNodeID();
                NodeID srcNodeID = storeVFGNode->getPAGSrcNodeID();
                Set<NodeID>::iterator it;
                if (taintedPagNodeIDs.find(srcNodeID) == taintedPagNodeIDs.end() &&
                    (it = taintedPagNodeIDs.find(dstNodeID)) != taintedPagNodeIDs.end())
                {
#if DEBUG
                    cout << "UNTAINT this node: " << dstNodeID << endl;
#endif
                    taintedPagNodeIDs.erase(it);
                    sinks.insert(storeVFGNode->getICFGNode()->getId());
#if DEBUG
                    cout << "store, find a sink:" << storeVFGNode->getICFGNode()->toString() << endl;
#endif
                }
                else if (taintedPagNodeIDs.find(srcNodeID) != taintedPagNodeIDs.end())
                {
#if DEBUG
                    cout << "taint this node:" << dstNodeID << endl;
#endif
                    taintedPagNodeIDs.insert(dstNodeID);
                }
            }
            //[外带的指针，传播规则]
            // q = p.f,如果p是tainted，p.f也是tainted，读取p的域到q，q应该被标记为tainted
            else if (GepSVFGNode::classof(succNode))
            {
                GepSVFGNode *gepSVFGNode = (GepSVFGNode *)succNode;
                NodeID srcNodeID = gepSVFGNode->getPAGSrcNodeID();
                if (taintedPagNodeIDs.find(srcNodeID) != taintedPagNodeIDs.end())
                {
                    NodeID dstNodeID = gepSVFGNode->getPAGDstNodeID();
#if DEBUG
                    cout << "taint this node:" << dstNodeID << endl;
#endif
                    taintedPagNodeIDs.insert(dstNodeID);
                }
            }
            //[外带的指针，传播规则]
            // p = &o,如果o是tainted，那么p也应该是tainted
            else if (AddrSVFGNode::classof(succNode))
            {
                AddrSVFGNode *addrSVFGNode = (AddrSVFGNode *)succNode;
                NodeID srcNodeID = addrSVFGNode->getPAGSrcNodeID();
                if (taintedPagNodeIDs.find(srcNodeID) != taintedPagNodeIDs.end())
                {
                    NodeID dstNodeID = addrSVFGNode->getPAGDstNodeID();
#if DEBUG
                    cout << "taint this node:" << dstNodeID << endl;
#endif
                    taintedPagNodeIDs.insert(dstNodeID);
                }
            }
            else
            {
#if DEBUG
                cout << "unsupported instruction:" << succNode->toString() << endl;
#endif
            }
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }
    std::cout << endl;
}

void ELA::getNextLoadNodes(NodeID startIcfgNodeId, NodeID pagNodeId, set<NodeID> &loadTaints)
{
    ICFGNode *iNode = icfg->getGNode(startIcfgNodeId);
    FIFOWorkList<const ICFGNode *> worklist;
    Set<const ICFGNode *> visited;
    worklist.push(iNode);
    while (!worklist.empty())
    {
        const ICFGNode *iNode = worklist.pop();
        for (ICFGNode::const_iterator it = iNode->OutEdgeBegin(), eit =
                                                                      iNode->OutEdgeEnd();
             it != eit; ++it)
        {
            ICFGEdge *edge = *it;
            ICFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
                if (IntraICFGNode::classof(succNode))
                {
                    IntraICFGNode *intraICFGNode = (IntraICFGNode *)succNode;
                    if (LoadInst::classof(intraICFGNode->getInst()))
                    {
                        LoadInst *loadInst = (LoadInst *)intraICFGNode->getInst();

                        if (pag->getValueNode(loadInst->getOperand(0)) == pagNodeId)
                        {
                            loadTaints.insert(pag->getValueNode(loadInst));
                        }
                    }
                }
            }
        }
    }
}

void ELA::findSinks()
{
    // exclude some files.
    std::string excludeFileName("enclave_t.c");
    auto needProcessNum = callgraph->getTotalNodeNum();
    for (auto it = callgraph->begin(), iEnd = callgraph->end(); it != iEnd; it++)
    {
        std::cout << needProcessNum-- << " ";
        if(needProcessNum == 123){
            continue;
        }
        PTACallGraphNode *ptaCallGraphNode = it->second;
        auto func = ptaCallGraphNode->getFunction();
        for (auto param : leakParams.getLeakParams())
        {
            if (func->getName().compare(param.getFuncName()) == 0)
            {
                if (param.getLeakType().compare("ECALL_OUT") == 0 ||
                    param.getLeakType().compare("ECALL_USER_CHECK") == 0 ||
                    param.getLeakType().compare("ECALL_ADT") == 0)
                {

                    auto funEntryICFGNode = icfg->getFunEntryICFGNode(func);
                    string funEntryICFGNodeStr = funEntryICFGNode->toString();
                    if (funEntryICFGNodeStr.find(excludeFileName) != std::string::npos)
                    {
#if DEBUG
                        cout << "NO need to process this icfgNode: " << funEntryICFGNodeStr << endl;
#endif
                        break;
                    }

                    auto var = funEntryICFGNode->getFormalParms()[param.getParamIndex()];
                    if (var)
                    {
                        NodeID pagNodeId = pag->getValueNode(var->getValue());
                        taintedPagNodeIDs.insert(pagNodeId);
                        auto svfgNode = svfg->getDefSVFGNode(pag->getGNode(pagNodeId));
                        auto nodeStr = svfgNode->toString();
                        ptrTaint(svfgNode);
                    }
                }
                // write to a pointer returned from an OCALL is a leak risk
                else if (param.getLeakType().compare("OCALL_RETURN") == 0)
                {
                    // find all callers
                    PTACallGraphEdge::CallInstSet csSet;
                    callgraph->getAllCallSitesInvokingCallee(ptaCallGraphNode->getFunction(), csSet);
                    for (auto callICFGNode : csSet)
                    {
                        auto var = callICFGNode->getActualParms()[param.getParamIndex()];
                        set<NodeID> taints;
                        auto pagNodeId = pag->getValueNode(var->getValue());
                        // e.g. %p = load(int* , %ActualParm),we mark %p as tainted.
                        // because the attacker may modify the content(is a pointer) in %ActualParm
                        // %p may point to untrusted memory.
                        getNextLoadNodes(callICFGNode->getId(), pagNodeId, taints);
                        if (taints.size() > 0)
                        {
                            for (auto id : taints)
                            {
                                taintedPagNodeIDs.insert(id);
                                auto svfgNode = svfg->getDefSVFGNode(pag->getGNode(id));
                                ptrTaint(svfgNode);
                            }
                        }
                    }
                }
                else if (param.getLeakType().compare("OCALL_IN") == 0 ||
                         param.getLeakType().compare("OCALL_IN_NON_PTR") == 0)
                {
                    PTACallGraphEdge::CallInstSet csSet;
                    callgraph->getAllCallSitesInvokingCallee(ptaCallGraphNode->getFunction(), csSet);
                    for (auto cs : csSet)
                    {
#if DEBUG
                        cout << "OCALL_IN, find a sink: " << icfg->getGNode(callId)->toString() << endl;
#endif
                        sinks.insert(cs->getId());
                        addParamIndex2sink(cs->getId(), param.getParamIndex());
                    }
                }
            }
        }
    }
}
// printSinks();
// }
//         else if (IntraICFGNode::classof(iNode))
//         {
//             // IntraBlockNode* intraICFGNode = (IntraBlockNode*)iNode;
//             // if(GetElementPtrInst::classof(intraICFGNode->getInst())){
//             //     for(auto vfgNode : intraICFGNode->getVFGNodes()){
//             //         if(GepVFGNode::classof(vfgNode)){
//             //             GepVFGNode* gepVFGNode = (GepVFGNode*)vfgNode;
//             //             NodeID src = gepVFGNode->getPAGSrcNodeID();
//             //             auto allocsites = ander->getPts(src);
//             //             for(auto site : allocsites){
//             //                 cout << pag->getGNode(site)->toString() << endl;
//             //                 auto v = pag->getGNode(site)->getValue();
//             //                 if(AllocaInst::classof(v)){
//             //                     AllocaInst* allocaInst = (AllocaInst*)v;
//             //                     // IntegerType::classof(allocaInst->getArraySize()->getType())
//             //                     allocaInst->print(errs());
//             //                     auto res = dyn_cast<ConstantInt>(allocaInst->getArraySize());
//             //                     cout << res->getZExtValue() << endl;

//             //                 }else{
//             //                     cout << "not allocaInst!" << endl;
//             //                 }
//             //             }

//             //         }else{
//             //             cout << "not GepVFGNode" << endl;
//             //         }
//             //     }
//             // }
//         }
//     }
// printSinks();
// }
// void ELA::backwardTracking()
// {
//     for (auto svfgNodeId : sinks)
//     {
//         auto svfgNode = svfg->getGNode(svfgNodeId);
//         if (StoreSVFGNode::classof(svfgNode))
//         {
//             auto storeSVFGNode = (StoreSVFGNode *)svfgNode;
//             auto srcPagId = storeSVFGNode->getPAGSrcNodeID();
//             //一个sink点,eg store(src,dst)，可能有多条incoming的direct边，
//             //需要过滤掉dst关联的direct边
//             for (auto it = storeSVFGNode->directInEdgeBegin(), end = storeSVFGNode->directInEdgeEnd(); it != end; it++)
//             {
//                 auto preNode = (SVFGNode *)(*it)->getSrcNode();
//                 // get pag node from preNode
//                 auto value = preNode->getValue();
//                 if (!value)
//                 {
// #if DEBUG
//                     cout << "getValue() is null:" << preNode->toString() << endl;
// #endif
//                     continue;
//                 }
//                 auto tmpPagId = pag->getValueNode(value);
//                 if ((*it)->isDirectVFGEdge() && srcPagId == tmpPagId)
//                 {
// #if DEBUG
//                     cout << "处理:" << preNode->toString() << endl;
// #endif
//                     std::set<const SVFGNode *> visited = {storeSVFGNode};
//                     std::vector<const SVFGNode *> path = {storeSVFGNode};
//                     DFS(visited, path, preNode);
//                 }
//             }
//         }
//     }
// }

void ELA::backwardTracking()
{
    for (auto icfgId : sinks)
    {
        auto iNode = icfg->getGNode(icfgId);
        if (CallICFGNode::classof(iNode))
        {
            CallICFGNode *callIcfgNode = (CallICFGNode *)iNode;
            std::cout << getParamIndexBySink(icfgId) << endl;
            auto param = callIcfgNode->getActualParms()[getParamIndexBySink(icfgId)];
            auto dstPagId = pag->getValueNode(param->getValue());
            auto startNode = svfg->getDefSVFGNode(pag->getGNode(dstPagId));
            std::set<const SVFGNode *> visited = {startNode};
            std::vector<const SVFGNode *> path = {startNode};
            DFS(visited, path, startNode);
            printPath(path);
        }
    }
}

void ELA::printPath(std::vector<const SVFGNode *> &path)
{
    std::cout << endl
              << endl;
    for (auto node : path)
    {
        std::cout << node->getId() << node->toString() << " ===> ";
    }
    std::cout << endl
              << endl;
}

void ELA::DFS(std::set<const SVFGNode *> &visited, std::vector<const SVFGNode *> &path, const SVFGNode *src)
{
    path.push_back(src);
    visited.insert(src);

    if (src->getInEdges().size() == 0)
    {
#if DEBUG
        cout << "开始回退:" << src->toString() << endl;
#endif
        printPath(path);
        path.pop_back();
        visited.erase(src);
        return;
    }

    for (auto edge : src->getInEdges())
    {
        if (visited.find(edge->getSrcNode()) == visited.end())
        {
            DFS(visited, path, edge->getSrcNode());
        }
    }

    path.pop_back();
    visited.erase(src);
}

void ELA::printICFGTraverseTime() {}

void printUseTime(string desc, clock_t start)
{
    clock_t end = clock();
    std::cout << desc << " :" << (double)(end - start) / CLOCKS_PER_SEC << endl;
}

void tmp(ELA *ela)
{
    auto icfg = ela->getICFG();
    for (auto it = icfg->begin(), iEnd = icfg->end(); it != iEnd; it++)
    {
        auto iNode = it->second;
        std::cout << "icfg node: " << iNode->toString() << endl;

        auto stmts = iNode->getSVFStmts();
        for (auto stm : stmts)
        {
            auto v = stm->getValue();
            std::cout << "Loc:" << getSourceLoc(v) << endl;
        }

        std::cout << "NEXT" << endl;

        // cout << "its snodes: " << endl;
        // auto sNodes = iNode->getVFGNodes();
        // cout << "size: " << sNodes.size() << endl;
        // // for (auto sNode : sNodes)
        // {
        //     cout << "sNode: " << sNode->toString() << endl;
        // }
    }
}

int main(int argc, char **argv)
{
    clock_t start, end;
    start = clock();
    std::cout << "start..." << endl;

#if REGRESSION
    regressionTest();
#endif
    // 1. sgx_wechat_app
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/sgx_wechat_app.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/sgx_wechat_app.config");
    // 2. sgx-based-mix-networks
    //  std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/sgx-based-mix-networks.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/sgx-based-mix-networks.config");
    // 3. sgx-dnet
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/sgx-dnet.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/sgx-dnet.config");
    // 4. sgx-aes-gcm
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/sgx-aes-gcm.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/sgx-aes-gcm.config");
    // 5. password-manager
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/password-manager.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/password-manager.config");
    // 6. TACIoT
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/TACIoT.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/TACIoT.config");
    // 7. Town-Crier
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/Town-Crier.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/Town-Crier.config");
    // 8. SGX_SQLite
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/SGX_SQLite.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/SGX_SQLite.config");

    // 9. mbedtls_SGX
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/mbedtls_SGX.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/mbedtls_SGX.config");

    // 10. Fidelius
    //  std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/Fidelius.ll"};
    //  ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/Fidelius.config");

    // just for test
    // std::vector<std::string> mnv = {"/home/yang/ELA/testcase/2.ll"};
    // ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/test.config");

    // 11. BiORAM-SGX
    //  std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/BiORAM-SGX.ll",
    //  "/home/yang/ELA/testcase/prod/BiORAM-SGX-dataowner_data.ll"};
    //  ELA *ela = new ELA(mnv, "/home/yang/ELA/src/config/BiORAM-SGX.config");

    // 12. PrivacyGuard
    //  std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/PrivacyGuard/CEE.ll",
    //  "/home/yang/ELA/testcase/prod/PrivacyGuard/DataBroker.ll",
    //  "/home/yang/ELA/testcase/prod/PrivacyGuard/Enclave_testML.ll",
    //  "/home/yang/ELA/testcase/prod/PrivacyGuard/iDataAgent.ll"};
    //  ELA *ela = new ELA(mnv, "/home/yang/ELA/testcase/prod/config/PrivacyGuard.config");

     // 12. talos
    //  std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/talos.ll"};
    //  ELA *ela = new ELA(mnv, "/home/yang/ELA/testcase/prod/config/talos.config");

    //13.SGX-Tor
    //  std::vector<std::string> mnv = {"/home/yang/ELA/testcase/prod/SGX-Tor.ll"};
    //  ELA *ela = new ELA(mnv, "/home/yang/ELA/testcase/prod/config/SGX-Tor.config");

    std::vector<std::string> moduleNameVec = {string(argv[1])};
    ELA *ela = new ELA(moduleNameVec, string(argv[2]));
    ela->findMallocNotCheck();
    ela->findSinks();
    set<string> rst;
    ela->getSinksSourceLocations(rst);
    std::cout << "result:" << endl;
    for (auto i : rst)
    {
        cout << i << endl;
    }

    // ela->backwardTracking();
    // ela->detectNullUseAnderson();

    printUseTime("total time", start);
    return 0;
}
