#! /usr/bin/python3.7
"""
@file
    MIB_Returnvalues.py
@brief
    Part of the Mission Information Base Exporter for the SOURCE project by KSat.
    TODO: Integrate into Parser Structure instead of calling this file (no cpp file generated yet)
@details
    Returnvalue exporter.

    To use MySQLdb, run pip install mysqlclient or install in IDE.
    On Windows, Build Tools installation might be necessary.

@data
    21.11.2019
"""
import pprint
import re
import os
import getpass
# import MySQLdb
from utility.mib_csv_writer import CsvWriter

doExportToFile = True
moveCsvFile = True
csvFilename = "MIB_Returnvalues.csv"
csvMoveDestination = "../"
fileSeparator = ';'
maxStringLength = 25
pp = pprint.PrettyPrinter(indent=0, width=250)


def main():
    print("Parsing Returnvalues: ")
    parseOBSW()
    handleFileExport()
    print("")


def parseOBSW():
    idInterfaceDefinitions = \
        parseInterfaceDefinitionFile("../../fsfw/returnvalues/FwClassIds.h",
                                     "../../config/returnvalues/classIds.h")
    print("Found interface definitions: ")
    pp.pprint(idInterfaceDefinitions)

    myHeaderList = getHeaderFileList("../../mission/")
    myHeaderList = myHeaderList + getHeaderFileList("../../fsfw/")
    myHeaderList = myHeaderList + getHeaderFileList("../../config/")
    myHeaderList = myHeaderList + getHeaderFileList("../../sam9g20/")
    mySecondList = parseHeaderFiles(idInterfaceDefinitions, myHeaderList)
    print(len(myHeaderList))
    print(len(mySecondList))
    # print(mySecondList[14081])
    # print (mySecondList.items()[0][1])
    # print( "Found entries:" )
    counter = 0
    # for entry in sorted(mySecondList):
    # print(entry)
    # noinspection PyUnusedLocal
    for entry in mySecondList.items():
        counter = counter + 1
    #    print( entry[0], entry[1][0], entry[1][1] )
    print("Count: ", counter)
    if doExportToFile:
        exportToFile(csvFilename, mySecondList)
    else:
        print('No export to file requested.')
    # writeEntriesToOtherDB( mySecondList )
    # writeEntriesToDB( mySecondList )


def handleFileExport():
    csvWriter = CsvWriter(csvFilename)
    if moveCsvFile:
        csvWriter.move_csv(csvMoveDestination)


def parseInterfaceDefinitionFile(fwFilename, missionFilename):
    file = open(fwFilename, "r")
    interfaces = dict()
    allLines = file.readlines()
    count = 0
    countMatched = False
    # Parses first entry, which has explicit value 1
    for line in allLines:
        if not countMatched:
            match = re.search('[\s]*([A-Z_0-9]*) = ([0-9]*),[\s]*//([A-Z]{1,3})', line)
        else:
            match = re.search('[\s]*([A-Z_0-9]*),[\s]*//([A-Z]{1,3})', line)
        if match and not countMatched:
            count = int(match.group(2))
            interfaces.update({match.group(1): [1, match.group(3)]})
            count += 1
            countMatched = True
        elif match:
            interfaces.update({match.group(1): [count, match.group(2)]})
            count += 1

    file = open(missionFilename, "r")
    allLines = file.readlines()
    for line in allLines:
        match = re.search('[\s]*([A-Z_0-9]*) = FW_CLASS_ID_COUNT,[\s]*(//([A-Z]{1,3}))?', line)
        if match:
            interfaces.update({match.group(1): [count, match.group(2)]})
            count += 1
    for line in allLines:
        match = re.search('^[\s]*([A-Z_0-9]*)[,]*[\s]*//[!<]*[\s]*([^\n]*)', line)
        if match:
            interfaces.update({match.group(1): [count, match.group(2)]})
            count += 1

    print("Found interfaces : " + str(count - 1))
    return interfaces


def returnNumberFromString(aString):
    if aString.startswith('0x'):
        return int(aString, 16)
    elif aString.isdigit():
        return int(aString)
    else:
        print('Error: Illegeal number representation: ' + aString)
        return 0


def convert(name):
    singleStrings = name.split('_')
    newString = ''
    for oneString in singleStrings:
        oneString = oneString.lower()
        oneString = oneString.capitalize()
        newString = newString + oneString
    return newString


def buildCheckedString(firstPart, secondPart):
    myStr = firstPart + convert(secondPart)
    if len(myStr) > maxStringLength:
        print("Error: Entry: " + myStr + " too long. Will truncate.")
        myStr = myStr[0:maxStringLength]
    else:
        # print("Entry: " + myStr + " is all right.")
        pass
    return myStr


def cleanUpDescription(descrString):
    description = descrString.lstrip('!<- ')
    if description == '':
        description = ' '
    return description


def parseHeaderFiles(interfaceList, fileList):
    dictionnary = dict()
    count = 0
    currentName = ""
    myId = 0
    dictionnary.update({0: ('OK', 'System-wide code for ok.', 'RETURN_OK', 'HasReturnvaluesIF.h',
                            'HasReturnvaluesIF')})
    dictionnary.update({1: ('Failed', 'Unspecified system-wide code for failed.',
                            'RETURN_FAILED', 'HasReturnvaluesIF.h', 'HasReturnvaluesIF')})
    print('')
    print("Parsing files: ")
    for fileName in fileList:
        # print("Parsing file " + fileName + ": ")
        file = open(fileName, "r")
        oldline = file.readline()
        while True:
            currentFullName = ""
            newline = file.readline()
            if not newline:
                break  # EOF
            if not oldline == '\n':
                twoLines = oldline + ' ' + newline.strip()
            else:
                twoLines = ''
            match1 = re.search('INTERFACE_ID[\s]*=[\s]*CLASS_ID::([a-zA-Z_0-9]*)', twoLines)
            if match1:
                # print("Interface ID" + str(match1.group(1)) + "found in " + fileName)
                currentId = interfaceList[match1.group(1)][0]
                currentName = interfaceList[match1.group(1)][1]
                currentFullName = match1.group(1)
                # print( "Current ID: " + str(currentId) )
                myId = currentId
            match = re.search('^[\s]*static const ReturnValue_t[\s]*([a-zA-Z_0-9]*)[\s]*=[\s]*'
                              'MAKE_RETURN_CODE[\s]*\([\s]*([x0-9a-fA-F]{1,4})[\s]*\);[\t ]*(//)?([^\n]*)',
                              twoLines)
            if match:
                # valueTable.append([])
                description = cleanUpDescription(match.group(4))
                stringToAdd = buildCheckedString(currentName, match.group(1))
                fullId = (myId << 8) + returnNumberFromString(match.group(2))
                if fullId in dictionnary:
                    print('duplicate returncode ' + hex(fullId) + ' from ' + fileName + ' was already in ' +
                          dictionnary[fullId][3])
                dictionnary.update({fullId: (stringToAdd, description, match.group(1), fileName, currentFullName)})
                #                 valueTable[count].append(fullId)
                #                 valueTable[count].append(stringToAdd)
                count = count + 1
            else:
                pass
            oldline = newline
    #     valueTable.pop()
    return dictionnary


def getHeaderFileList(base):
    # print ( "getHeaderFileList called with" + base )
    baseList = os.listdir(base)
    fileList = []
    for entry in baseList:
        # Remove all hidden files:
        if os.path.isdir(base + entry) and (entry[0] != ".") and (entry[0] != "_"):
            fileList = fileList + getHeaderFileList(base + entry + "/")
        if re.match("[^.]*\.h", entry) and os.path.isfile(base + entry):
            fileList.append(base + entry)
    return fileList


def writeEntriesToDB(listOfEntries):
    print("Connecting to database...")
    user = getpass.getpass("User: ")
    passwd = getpass.getpass()
    conn = MySQLdb.connect(host="127.0.0.1", user=user, passwd=passwd, db="flpmib")
    written = conn.cursor()
    print("done.")
    # delete old entries
    print("Kill old entries.")
    written.execute("DELETE FROM txp WHERE TXP_NUMBR = 'DSX00000'")
    print("Insert new ones:")
    for entry in listOfEntries.items():
        written.execute("INSERT INTO txp (txp_numbr, txp_from, txp_to, txp_altxt) VALUES ('DSX00000', %s, %s, %s)",
                        [entry[0], entry[0], entry[1][0]])
    conn.commit()
    print("Done. That was easy.")


def writeEntriesToOtherDB(listOfEntries):
    print("Connecting to other database...")
    conn = MySQLdb.connect(host="buggy.irs.uni-stuttgart.de",
                           user='returncode', passwd='returncode', db="returncode")
    written = conn.cursor()
    print("connected.")
    # delete old entries
    print("Kill old entries.")
    written.execute("DELETE FROM returncodes WHERE true")
    print("Insert new ones:")
    for entry in listOfEntries.items():
        written.execute("INSERT INTO returncodes (code,name,interface,file,description) VALUES (%s, %s, %s, %s, %s)",
                        [entry[0], entry[1][2], entry[1][4], entry[1][3], entry[1][1]])
    conn.commit()
    print("Done. That was hard.")


def exportToFile(filename_, listOfEntries):
    print('Exporting to file: ' + csvFilename)
    file = open(filename_, "w")
    for entry in listOfEntries.items():
        file.write(hex(entry[0]) + fileSeparator + entry[1][0] + fileSeparator + entry[1][1] +
                   fileSeparator + entry[1][2] + fileSeparator
                   + entry[1][3] + fileSeparator + entry[1][4] + '\n')
    file.close()
    return


if __name__ == "__main__":
    main()
