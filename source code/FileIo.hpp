#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string>

class FileIo
{
public:
    FileIo(std::string file_name);
    bool sync();

private:
    const static bool kFailure = false;
    const static bool kSuccess = true;
    int file;
    char log_file_name[20];
};
