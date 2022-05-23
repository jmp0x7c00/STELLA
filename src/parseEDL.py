import re
FUNCNAME_REG = re.compile(r'([a-z|A-Z|0-9]|_)+$')
filename = "./enclave.edl"
lines = []
with open(filename) as file:
    lines = file.readlines()
isTrusted = False
isUntrusted = False
id = 0
record_template = "{}   {}    {}    {}"
for line in lines:
    line = line.strip()
    if "trusted {" in line and "untrusted {" not in line:
        isTrusted = True

    if isTrusted and ("};" in line):
        isTrusted = False

    if isTrusted and "[user_check]" in line:
        tmp = line.split("(")
        funcName = FUNCNAME_REG.search(tmp[0].strip()).group();
        argArr = tmp[1].split(",")
        for argIndex in range(len(argArr)):
            if "[user_check]" in argArr[argIndex]:
                id += 1
                print(record_template.format(id,funcName,argIndex,"ECALL_USER_CHECK"))