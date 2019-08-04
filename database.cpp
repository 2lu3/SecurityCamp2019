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
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::vector;

#define FOR for
#define rep(i, n) FOR(int64_t i = 0; i < n; ++i)
// 現在の関数名を取得
#define FUNCNAME __FUNCTION__

DataBase::DataBase()
{
    // Record初期化

    for (int i = 0; i < table_num; ++i)
    {
        table[i].id = 0;
    }

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

// id : 変更する前のRecordのid
// update_record_condition : 変更後のRecord
// target_recordを使用して、tableでの添字を取得し、update_record_conditionを代入する
int DataBase::updateRecord(uint64_t id, const Record &update_record_condition)
{
    // idが変更されていないかチェック
    if (id != update_record_condition.id)
    {
        cerr << FUNCNAME << "(): id of target_record and update_record_condition is not same" << endl;
        return kFailure;
    }

    // update_record_conditionが制約に反していないかチェックする
    if (checkRecord(update_record_condition, CHECK_RECORD_OPTION_UPDATE) == kFailure)
    {
        return kFailure;
    }

    // idから、primary_indexを使って、table[]の添字を取得
    uint32_t target_table_index = primary_index[id];
    // 変更する対象の添字に、update_record_conditionを代入
    table[target_table_index] = update_record_condition;
    return kSuccess;
}
// target_record : 変更する前のRecord
// update_record_condition : 変更後のRecord
// target_recordを使用して、tableでの添字を取得し、update_record_conditionを代入する
int DataBase::updateRecord(const Record &target_record, const Record &update_record_condition)
{
    if (target_record.id == 0)
    {
        return kFailure;
    }
    return updateRecord(target_record.id, update_record_condition);
}

// check_recordで与えられたRecordが制約に収まっているかチェックする
// option: チェックする内容がRecordのinsertとupdateで微妙に違う 0:insert 1:update
int DataBase::checkRecord(const Record &check_record, int option)
{
    // 制約1: idはユニークである
    // insertのときのみ
    if (option == CHECK_RECORD_OPTION_INSERT)
    {
        // idがすでに登録されている場合
        if (primary_index.count(check_record.id) != 0)
        {
            return kFailure;
        }
    }

    // 制約2: idは0以外である
    // insertのときは、自動でidを割り振る
    if (option == CHECK_RECORD_OPTION_INSERT)
    {
        int random_id = rnd();
        while (primary_index.count(random_id) == 1)
        {
            random_id = rnd();
        }
    }
    else // option == CHECK_RECORD_OPTION_UPDATE
    {
        if (check_record.id == 0)
        {
            return kFailure;
        }
    }

    // 制約3: columnに登録されている(map型での)keyの集合は、column_namesに格納されている文字列の集合と等価でなければならない
    // insert, update 両方

    // check_recordにあるが、column_namesにはないものを探す
    for (const auto &[key, value] : check_record.columns)
    {
        // column_namesに登録されているかチェック
        if (column_names.count(key) != 1)
        {
            cerr << "key '" << key << "' is not registered in column_names" << endl;
            return kFailure;
        }
    }

    // column_namesにあるが、check_recordにはないものを探す
    for (const auto &key : column_names)
    {
        // check_recordに登録されているかチェック
        if (check_record.columns.count(key) != 1)
        {
            cerr << "key '" << key << "' is not registered in record" << endl;
            return kFailure;
        }
    }

    return kSuccess;
}

// target_recordで指定したRecordを削除する
int DataBase::deleteRecord(const Record &target_record)
{
    return deleteRecord(target_record.id);
}
// target_recordで指定したRecordを削除する
int DataBase::deleteRecord(uint64_t id)
{
    // target_recordのidから
    uint32_t target_table_index = primary_index[id];
    for (int i = target_table_index; i < table_num - 1; ++i)
    {
        table[i] = table[i + 1];
    }
    --table_num;
    primary_index.erase(target_table_index);
    return kSuccess;
}

// table[]の末尾に新しいRecordを追加
int DataBase::insertRecord(const Record &new_record)
{
    // Recordの制約チェック
    if (checkRecord(new_record, CHECK_RECORD_OPTION_INSERT) == kFailure)
    {
        cerr << FUNCNAME << "(): Failed to insert Record" << endl;
        return kFailure;
    }
    table[table_num] = new_record;
    primary_index[new_record.id] = table_num;
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
