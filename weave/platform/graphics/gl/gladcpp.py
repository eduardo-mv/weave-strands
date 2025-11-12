import sys
from enum import Enum

gladcppHeader = """
#pragma once

/*
Auto generated C++ namespaced wrappers for glad generated GL headers
*/

#include "glad.h"

namespace gl {
{ENUMS}

{API}
}

"""

gladwglcppHeader = """
#pragma once

/*
Auto generated C++ namespaced wrappers for glad generated WGL headers
*/
#include "glad_wgl.h"

namespace wgl {
{ENUMS}

{API}
}

"""

enumOpener = ["#define GL_", "#define WGL_"]
enumIgnore = ["GL_EXT", "GL_VERSION"]
enumFix = ["FALSE", "TRUE", "NO_ERROR", "WAIT_FAILED"]
enumList = []

apiEntry = ["(APIENTRYP", "#define gl", "#define wgl"]
currentApiEntry = ""
apiEntryList = []

def findIn(line, list):
    for a in list:
        if a in line:
            return True
    return False

def EnumParse(line):
    if len(enumList) > 0:
        enumList[-1] = enumList[-1].replace("\n",",\n")
    for opener in enumOpener:
        line = line.replace(opener, "    ")
    enumEntry = line.replace(" 0", " = 0").replace(" 1", " = 1")
    for fix in enumFix:
        enumEntry = enumEntry.replace(fix, fix + "_")
    enumList.append(enumEntry)

def ApiParse(line):
    global currentApiEntry
    if currentApiEntry == "":
        currentApiEntry = line
    elif "glad_" in line:
        apiProc = currentApiEntry
        apiName = line[line.index("glad_"):].replace(";\n","")
        apiPFN = apiProc[apiProc.index("("):apiProc.index(")")+1]
        apiParams = apiProc.split(")(")

        for param in apiParams:
            if ");" in param:
                apiParams = param.replace(");\n", "").replace("const","").replace("*", "").split(" ")
                apiParams = [entry for entry in apiParams if len(entry) > 0]
                apiParams = [entry for i, entry in enumerate(apiParams) if i % 2 != 0]
                break
        
        apiCall = "{ return " + apiName + "(" + ("".join(apiParams)).replace(",", ", ") + "); }"

        apiLine  = apiProc.replace("typedef", "inline").replace(apiPFN, apiName.replace("glad_gl", "").replace("glad_wgl", "")).replace(");", ")" + apiCall).replace("\n", "") + "\n"
        apiEntryList.append(apiLine)
        currentApiEntry =   ""

def ParseGlad(gladFile, body):
    global enumList
    global apiEntryList

    enumList = []
    apiEntryList = []

    for line in gladFile:
        if findIn(line, enumOpener) and not findIn(line, enumIgnore):
            EnumParse(line)
        elif findIn(line, apiEntry):
            ApiParse(line)
    

    enums = "enum {\n" + "".join(enumList) + "\n};"
    apis = "".join(apiEntryList)

    return body.replace("{ENUMS}", enums).replace("{API}", apis)



filename = "glad.h"

with open(filename) as file:
    out = ParseGlad(file, gladcppHeader)
    with open("gladc++.h", "w") as file:
        file.write(out)

filename = "glad_wgl.h"

with open(filename) as file:
    out = ParseGlad(file, gladwglcppHeader)
    with open("gladc++_wgl.h", "w") as file:
        file.write(out)
