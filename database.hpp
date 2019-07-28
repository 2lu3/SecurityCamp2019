#ifndef SECURITYCAMP2019_DATABASE_HPP_
#define SECURITYCAMP2019_DATABASE_HPP_

#include <cstdint>
#include <string>
#include <iostream>
#include <map>
#include <random>
#include <set>

class DataBase
{
public:
    struct Record
    {
        // column
        std::map<std::string, std::string> columns;
    };
    const static int kSuccess = 1;
    const static int kFailure = 0;
    DataBase();
    int crashRecovery();
    int begin();
    int readRecord(std::map<std::string, std::string> target_columns, std::vector<Record> *record_vector);
    int updateRecord();
    int deleteRecord();
    int insertRecord();
    int commit();
    int abort();
    int createKey(std::string columns[]);

private:
    const static int column_max_num = 10;
    const static int table_max_num = 100;
    const int data_num = 50;
    int column_num = 0;
    int table_num = 0;

    std::set<std::string> column_names;

    Record table[table_max_num];
    // key-Record
    std::multimap<std::string, Record> index_maps[column_max_num];
};

#endif //SECURITYCAMP2019_DATABASE_HPP_
