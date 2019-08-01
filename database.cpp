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
#define FUNCNAME __FUNCTION__

DataBase::DataBase()
{
    FILE *fp = fopen("data.csv", "r");
    if (fp == NULL)
    {
        cout << "ファイルが開きません" << endl;
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
    target_columnsで指定した条件にあうRecord(複数可)を返す関数
    用例:
    map<string, string> mp;
    mp["name"] = "山田";
    mp["gender"] = "male"; // 条件を指定
    vector<Record> record_receiver; // Recordを受け取るためのvector
    dataBase.readRecord(mp, record_receiver);
    for(auto &value : vec) {
        // valueは、mpで指定した条件に合うRecord
    }
*/
int DataBase::readRecord(const map<string, string> &target_columns, vector<Record> &return_records)
{
    // 絞り込んでいる最中/絞り込んだtableの添字の集合
    vector<int> selected_table_index;
    // 最初は、0~レコードの数まですべて
    rep(i, table_num)
    {
        selected_table_index.emplace_back(i);
    }

    // 複数の条件を、順番に確認していく
    for (const auto &[column_name, column_value] : target_columns)
    {
        // column_nameが存在するかチェック
        // column_namesには、現在使われているcolumnの名前がすべて格納されている
        if (column_names.count(column_name) == 1)
        {
            // 条件にあわないRecordの添字をselected_table_indexから消去していく
            for (auto it = selected_table_index.begin(); it != selected_table_index.end(); ++it)
            {
                // もし、値が一致しない場合は、条件に合っていないということ
                if (table[*it].columns[column_name] != column_value)
                {
                    // selected_table_indexから削除
                    selected_table_index.erase(it);
                }
            }
        }
        else
        {
            // 存在しないcolumnの名前を指定された場合
            cerr << "Error: " << FUNCNAME << "(): there is no column_name '" << column_name << "' in column_names" << endl;
            return kFailure;
        }
    }

    // return_recordsに、絞り込んだRecordを代入していく
    for (auto it = selected_table_index.begin(); it != selected_table_index.end(); ++it)
    {
        return_records.emplace_back(table[*it]);
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

// table[]の末尾に新しいRecordを追加
int DataBase::insertRecord(const Record &new_record)
{
    for (const auto &[key, value] : new_record.columns)
    {
        // column_namesに登録されているかチェック
        if (column_names.count(key) != 1)
        {
            cerr << "key '" << key << "' is not registered" << endl;
            return kFailure;
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
