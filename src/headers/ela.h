#ifndef ELA_H_
#define ELA_H_
#include "LeakParams.h"
#include <iostream>
#include "SVF-FE/LLVMUtil.h"
#include "Graphs/SVFG.h"
#include "WPA/Andersen.h"
#include "SVF-FE/SVFIRBuilder.h"
#include "Util/Options.h"
#include <time.h>
#include <map>
void printUseTime(string desc, clock_t start);
class ELA
{
private:
    SVFIR *pag;
    ICFG *icfg;
    SVFG *svfg;
    Andersen *ander;
    PTACallGraph *callgraph;
    set<NodeID> sinks;
    map<NodeID, unsigned> sinkAndParamIndexMapping;
    Set<NodeID> taintedPagNodeIDs;
    LeakParams leakParams;

public:
    ELA(std::vector<std::string> &mnv, string configFile)
    {
        clock_t initStart = clock();
        if (configFile.compare("") != 0)
        {
            leakParams.readFromConfigFile(configFile);
        }
        std::cout << "EDL param entry num: " << leakParams.getLeakParams().size() << std::endl;
        SVFModule *svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(mnv);
        svfModule->buildSymbolTableInfo();
        SVFIRBuilder builder;
        pag = builder.build(svfModule);
        ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
        SVFGBuilder svfBuilder;
        svfg = svfBuilder.buildFullSVFG(ander);
        icfg = pag->getICFG();
        // icfg->dump("icfg.dot");
        callgraph = ander->getPTACallGraph();
        // callgraph->dump("patCall.dot");
        printUseTime("initialization time", initStart);
    }
    ~ELA()
    {
        delete svfg;
        AndersenWaveDiff::releaseAndersenWaveDiff();
        SVFIR::releaseSVFIR();
        // LLVMModuleSet::getLLVMModuleSet()->dumpModulesToFile(".svf.bc");
        SVF::LLVMModuleSet::releaseLLVMModuleSet();
        llvm::llvm_shutdown();
    }
    inline ICFG *getICFG() { return icfg; }
    inline SVFG *getSVFG() { return svfg; }
    inline PAG *getPAG() { return pag; }
    inline PTACallGraph *getCG() { return callgraph; }
    void findSinks();
    void ptrTaint(const SVF::SVFGNode *);
    void getNextLoadNodes(NodeID, NodeID, set<NodeID> &);
    void traverseToFindCallNodeInsideFunction(NodeID start, string funcName, set<NodeID> &ret);
    void printSinks();
    void getSinksSourceLocations(set<string> &locs);
    void backwardTracking();
    void DFS(std::set<const SVFGNode *> &visited, std::vector<const SVFGNode *> &path, const SVFGNode *src);
    void printPath(std::vector<const SVFGNode *> &path);
    void printICFGTraverseTime();
    void detectNullUseAnderson();
    void findMallocNotCheck();
    void addParamIndex2sink(NodeID sink, unsigned paramIndex){sinkAndParamIndexMapping[sink] = paramIndex;}
    NodeID getParamIndexBySink(NodeID sink){return sinkAndParamIndexMapping[sink];}
    void DFS(set<const PTACallGraphNode*> &visited, vector<const PTACallGraphNode*> &path, const PTACallGraphNode* src, const PTACallGraphNode *dst);
};
#endif