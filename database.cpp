#include "database.hpp"

#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::extent;
using std::fscanf;
using std::int64_t;
using std::string;
using std::to_string;
using std::uint64_t;
using std::uint8_t;

#define FOR for
#define rep(i, n) FOR(int64_t i = 0; i < n; ++i)
// 現在の関数名を取得
#define FUNCNAME getFuncName(__FUNCTION__)

// const char*型からstring型へとcast
inline string getFuncName(const char *name)
{
    return name;
}

DataBase::DataBase()
{
    FILE *fp = fopen("data.csv", "r");
    if (fp == NULL)
    {
        cout << "ファイルが開きません" << endl;
    }
    int id, money;
    char name[10];
    Record value;
    rep(i, data_num)
    {
        fscanf(fp, "%d,%[^,],%d", &id, name, &money);
        value.id = id - 1;
        value.name = name;
        value.money = money;
        cout << value.id << " " << value.name << " " << value.money << endl;
        primary_index[value.id] = value;
        name_record_map[value.name] = value;
    }
    cout << name_record_map[name].name << endl;
    fclose(fp);
}

int DataBase::begin()
{
}

int DataBase::createKey(string columns[])
{
}

int DataBase::readRecord(string name, Record *return_record)
{
    cout << "hello" << endl;
    cout << "record " << name_record_map["中川"].name << endl;
    // *return_record = static_cast<Record>(name_record_map[name]);
    return 0;
}

int DataBase::updateRecord()
{
}

int DataBase::deleteRecord()
{
}

int DataBase::insertRecord()
{
}

int DataBase::commit()
{
}

int DataBase::abort()
{
}

int DataBase::crashRecovery()
{
}
