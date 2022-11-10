#ifndef LEAK_PARAMS_H
#define LEAK_PARAMS_H
#include "LeakParamItem.h"
#include <fstream>
#include <sstream>
class LeakParams
{
private:
    std::vector<LeakParamItem> params;

public:
    LeakParams() {}
    void readFromConfigFile(std::string filePath)
    {
        std::ifstream infile(filePath);
        std::string line;
        while (getline(infile, line))
        {
            if (line.find("#") != line.npos)
            {
                continue;
            }
            LeakParamItem item;
            std::istringstream iss(line);
            iss >> item;
            params.push_back(item);
        }
    }
    std::vector<LeakParamItem>& getLeakParams(){
        return params;
    }
};
#endif