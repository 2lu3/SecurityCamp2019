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
// Œ»Ý‚ÌŠÖ”–¼‚ðŽæ“¾
#define FUNCNAME getFuncName(__FUNCTION__)

// const char*Œ^‚©‚çstringŒ^‚Ö‚Æcast
inline string getFuncName(const char *name)
{
    return name;
}

DataBase::DataBase()
{
    FILE *fp = fopen("data.csv", "r");
    if (fp == NULL)
    {
        cout << "ƒtƒ@ƒCƒ‹‚ªŠJ‚«‚Ü‚¹‚ñ" << endl;
    }
    int id, money;
    char name[10];
    Record record;
    rep(i, data_num)
    {
        if (fscanf(fp, "%d,%[^,],%d", &id, name, &money) == EOF)
        {
            cerr << "Error: failed to fscanf" << endl;
        }
        record.id = id - 1;
        record.name = name;
        record.money = money;
        primary_index[record.id] = record;
        name_record_map[record.name] = record;
    }
    fclose(fp);
}

int DataBase::begin()
{
    return kSuccess;
}

int DataBase::createKey(string columns[])
{
    return kSuccess;
}

int DataBase::readRecord(string name, Record *return_record)
{
    *return_record = name_record_map[name];
    return 0;
}

int DataBase::updateRecord()
{
    return kSuccess;
}

int DataBase::deleteRecord()
{
    return kSuccess;
}

int DataBase::insertRecord()
{
    return kSuccess;
}

int DataBase::commit()
{
    return kSuccess;
}

int DataBase::abort()
{
    return kSuccess;
}

int DataBase::crashRecovery()
{
    return kSuccess;
}
