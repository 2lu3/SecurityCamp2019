#ifndef SECURITYCAMP2019_DATABASE_HPP_
#define SECURITYCAMP2019_DATABASE_HPP_

#include <cstdint>
#include <string>
#include <iostream>
#include <map>
#include <random>

class DataBase
{
public:
    struct Record
    {
        int id;
        int money;
        std::string name;
    };
    const static int kSuccess = 1;
    const static int kFailure = 0;
    DataBase();
    int crashRecovery();
    int begin();
    int readRecord(std::string name, Record *return_record);
    int updateRecord();
    int deleteRecord();
    int insertRecord();
    int commit();
    int abort();
    int createKey(std::string columns[]);

private:
    const int data_num = 50;

    std::map<int, Record> primary_index;
    std::map<std::string, Record> name_record_map;
};

#endif //SECURITYCAMP2019_DATABASE_HPP_
