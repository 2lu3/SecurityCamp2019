#include "FileIo.hpp"

#include <unistd.h>

FileIo::FileIo(std::string file_name)
{
    strcpy(log_file_name, file_name.c_str());
    // log_file_name = file_name;
    // file = open(file_name, O_WRONLY);
}

bool FileIo::sync()
{
    file = open(log_file_name, O_WRONLY);
    if (file == -1)
    {
        exit(1);
    }
    if (fsync(file) == -1)
    {
        exit(1);
    }
    close(file);
}
