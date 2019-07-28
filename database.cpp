#include "database.hpp"

#include <iostream>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::extent;
using std::fscanf;
using std::int64_t;
using std::map;
using std::string;
using std::to_string;
using std::uint64_t;
using std::uint8_t;
using std::vector;

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
    Record record;
    rep(i, data_num)
    {
        if (fscanf(fp, "%d,%[^,],%d", &id, name, &money) == EOF)
        {
            cerr << "Error: failed to fscanf" << endl;
        }
    }
    fclose(fp);
}

int DataBase::begin()
{
    return kSuccess;
}

int DataBase::createKey(std::string columns[])
{
    return kSuccess;
}

int DataBase::readRecord(map<string, string> target_columns, vector<Record> *record_vector)
{
    vector<int> vec;
    rep(i, table_num)
    {
        vec.emplace_back(i);
    }
    for (const auto &[key, value] : target_columns)
    {
        // keyが存在するかチェック
        if (column_names.count(key) == 1)
        {
            // 残っている候補を順番に確かめる
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                // もし、値が一致しない場合
                if (table[*it].columns[key] != value)
                {
                    // vecから削除
                    vec.erase(it);
                }
            }
        }
        else
        {
            cerr << "Error: " << FUNCNAME << "(): there is no key '" << key << "' in column_names" << endl;
            return kFailure;
        }
    }

    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        record_vector->emplace_back(table[*it]);
    }
    return kSuccess;
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

int DataBase::insertRecord(Record new_record)
{
    for (const auto &[key, value] : new_record.columns)
    {
        // column_namesに登録されているかチェック
        if (column_names.count(key) != 1)
        {
            new_record.columns.erase(key);
        }
    }
    table[table_num] = new_record;
    table_num++;
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
