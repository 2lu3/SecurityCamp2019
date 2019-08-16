#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class FileIo
{
public:
    FileIo(char file_name[]);

private:
    int file;
};
