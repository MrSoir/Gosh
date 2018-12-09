import zipfile
import sys
import os
        
def extractZipFile(tarZipFilePath, extractionPath):
    print('tarZipFilePath: ', tarZipFilePath)
    zipper = zipfile.ZipFile(tarZipFilePath, 'r', zipfile.ZIP_DEFLATED)
    zipper.extractall(extractionPath)
    zipper.close()

if __name__ == "__main__":
    for cntr, arg in enumerate(sys.argv):
        print(cntr, ": ", arg)

    if len(sys.argv) < 3:
        print("less than 2 arguemnts given! - exiting ", sys.argv[0]);
    else:
        tarZipFilePath = sys.argv[1]
        extractionPath = sys.argv[2]

        extractZipFile(tarZipFilePath, extractionPath)



