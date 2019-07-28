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

/*
    条件に合うRecordを返す
    target_columnsは、例えば<"name", "山田">のように、条件を指定する(複数可)
    record_vectorは、条件に合うRecord(複数可)を格納するためのポインタ渡し
*/
int DataBase::readRecord(map<string, string> target_columns, vector<Record> *record_vector)
{
    // 絞り込んでいる最中/絞り込んだtableの添字の集合
    // 最初は、0~table_numまですべて
    vector<int> vec;
    rep(i, table_num)
    {
        vec.emplace_back(i);
    }
    for (const auto &[column_name, column_value] : target_columns)
    {
        // column_nameが存在するかチェック
        if (column_names.count(column_name) == 1)
        {
            // 残っている候補を順番に確かめる
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                // もし、値が一致しない場合は、条件に合っていないということ
                if (table[*it].columns[column_name] != column_value)
                {
                    // vecから削除
                    vec.erase(it);
                }
            }
        }
        else
        {
            cerr << "Error: " << FUNCNAME << "(): there is no column_name '" << column_name << "' in column_names" << endl;
            return kFailure;
        }
    }

    // record_vectorに、絞り込んだtableを代入していく
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        record_vector->emplace_back(table[*it]);
    }
    return kSuccess;
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
