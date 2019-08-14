#include "database.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <set>

using std::cerr;
using std::cout;
using std::endl;
using std::extent;
using std::flush;
using std::ifstream;
using std::int64_t;
using std::map;
using std::ofstream;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::vector;

// 現在の関数名を取得
#define FUNCNAME __FUNCTION__

/* Record関連 */

DataBase::Record::Record()
{
    // idはnull値で初期化
    id = kIdNull;
}

/* DataBase関連 */

DataBase::DataBase() : redoLog(new RedoLog)
{
}

// 未実装
int DataBase::begin()
{
    return kSuccess;
}

// 未実装
int DataBase::createKey(std::string columns[])
{
    cout << columns[0] << endl;
    return kSuccess;
}

// check_recordで与えられたRecordが制約(database.hppに記載)に収まっているかチェックする
int DataBase::checkRecord(const Record &check_record)
{
    // 制約2: idはkIdNull以外である
    if (check_record.id == Record::kIdNull)
    {
        cerr << "Error " << FUNCNAME << "(): IDNULL" << endl;
        return kFailure;
    }

    // 制約3: columnに登録されている(map型での)keyの集合は、column_namesに格納されている文字列の集合と等価でなければならない
    // insert, update 両方
    // 最初に要素数を比べる
    if (column_names.size() != check_record.columns.size())
    {
        // 要素数が違う場合、等価でない
        cerr << "Error: " << FUNCNAME << "(): column_names.size() != columns.size()" << endl;
        return kFailure;
    }

    // 順番にkeyが一致するかを調べる
    // 初期化式の中で、for(auto column_names_iterator = column_names.begin(), auto column_iterator = check_record.columns.begin())とするとエラーが出る
    auto column_names_iterator = column_names.begin();
    auto column_iterator = check_record.columns.begin();
    for (;
         column_names_iterator != column_names.end() && column_iterator != check_record.columns.end();
         ++column_iterator, ++column_names_iterator)
    {
        // column_namesのkey と columnのkeyが一致していない場合
        if (*column_names_iterator != column_iterator->first)
        {
            cerr << "Error: " << FUNCNAME << "(): column_name(" << *column_names_iterator << ") != columns(" << column_iterator->first << ")";
            return kFailure;
        }
    }

    return kSuccess;
}

int DataBase::commit()
{
    // todo : ファイル書き込み

    // write_setの内容をprimary_indexに反映する
    for (auto itr = write_set.begin; itr != write_set.end(); ++itr)
    {
        primary_index[itr->first] = itr->second;
    }

    return kSuccess;
}

int DataBase::abort()
{
    // ファイルの内容を破棄するだけ
    redoLog->resetLogFile();
    return kSuccess;
}

// 未実装
int DataBase::crashRecovery()
{
    return kSuccess;
}

/* DataBase Delete, Insert, Read, Update */

// deleteRecord(uint64_t id)のオーバーロード
int DataBase::deleteRecord(const Record &target_record)
{
    return deleteRecord(target_record.id);
}

// target_recordで指定したRecordを削除する
int DataBase::deleteRecord(uint64_t id)
{
    // write_setにすでに登録済みか
    if (auto itr = write_set.find(id); itr != write_set.end())
    {
        // columnsを空にすることで消去を表す
        itr->second.columns.clear();
    }
    else
    {
        // write_setに空のRecordを追加
        write_set.insert(std::make_pair(id, Record()));
    }
    return kSuccess;
}

// table[]の末尾に新しいRecordを追加
// idを割り振るため、const Record &new_recordにしていない
int DataBase::insertRecord(Record &new_record)
{
    // primary_indexにもwrite_setにもないidをランダムで生成
    uint64_t id = rnd();

    auto primary_itr = primary_index.find(id);
    auto write_set_itr = write_set.find(id);

    while (primary_itr != primary_index.end() || write_set_itr != write_set.end() || id == Record::kIdNull)
    {
        id = rnd();
        primary_itr = primary_index.find(id);
        write_set_itr = write_set.find(id);
    }

    // write_setにidをセット
    new_record.id = id;

    if (checkRecord(new_record) == kFailure)
    {
        cerr << "Error: " << FUNCNAME << "(): checkRecord() error" << endl;
        return kFailure;
    }
    write_set[id] = new_record;

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
    /*
    writeset検索
    db検索
    merge
    x2

    merge

    x n-1 回
    */
    // return_recordsの初期化
    return_records.clear();

    // 2つ用意して、mergeする
    set<uint64_t> column_sets[2];
    int i = 0;

    for (const auto &column_name_value_pair : target_columns)
    {
        // write set検索
        // write set(map型)内のRecordをすべて回す
        // write_set_itr->second = write_set内のRecord
        for (auto write_set_itr = write_set.begin(); write_set_itr != write_set.end(); ++write_set_itr)
        {
            // Recordのcolumnsのcolumn_nameがcolumn_valueかどうか
            // itrは、columnsのiterator
            // if(auto itr = record.columns.find(column_name); itr != record.columns.end())
            if (auto itr = write_set_itr->second.columns.find(column_name_value_pair.first); itr != write_set_itr->second.columns.end())
            {
                // Record.column[column_name] == value
                if (itr->second == column_name_value_pair.second)
                {
                    // column_sets[iが0なら0、それ以外なら1]に追加
                    column_sets[i == 0].insert(write_set_itr->second.id);
                }
            }
        }

        // <column_name, column_value>のペアで、ID(複数・set型)を検索する
        if (auto column_index_itr = column_index.find(column_name_value_pair); column_index_itr != column_index.end())
        {
            // column_index_itr->second(set型)から、順番にIDを抽出
            for (auto const &id : column_index_itr->second)
            {
                // 抽出したidがwrite_setに存在するか
                // write_setに存在する場合、なにもしない
                if (auto write_set_itr = write_set.find(id); write_set_itr == write_set.end())
                {
                    // 存在しない場合のみ追加
                    // i==0のとき0,それ以外の時1にする
                    column_sets[i == 0].insert(id);
                }
            }
            if (i == 0)
            {
                // iが0のときは飛ばす
                continue;
            }
        }
        else
        {
            // todo; keyが正しい場合は、パス
            cerr << "Error: " << FUNCNAME << "(): column_name and value error" << endl;
            return kFailure;
        }
        auto itr0 = column_sets[0].begin();
        auto itr1 = column_sets[1].begin();
        while (itr0 != column_sets[0].end() && itr1 != column_sets[1].end())
        {
            if (*itr0 == *itr1)
            {
                ++itr0;
                ++itr1;
                continue;
            }
            else if (*itr0 < *itr1)
            {
                // 小さいほうは、==になることはないので、消す
                itr0 = column_sets[0].erase(itr0);
            }
            else
            {
                // *itr0 > *itr1
                // 小さいほうは、==になることはないので飛ばす
                // 保存するのは、itr0のほうだけ(消すのはもったないない)
                ++itr1;
            }
        }
        ++i;
    }

    for (auto const &id : column_sets[0])
    {
        if (auto write_set_itr = write_set.find(id); write_set_itr != write_set.end())
        {
            return_records.push_back(write_set_itr->second);
        }
        else
        {
            if (auto primary_index_itr = primary_index.find(id); primary_index_itr != primary_index.end())
            {
                return_records.push_back(primary_index_itr->second);
            }
            else
            {
                cerr << "Error " << FUNCNAME << "(): write set and primary_index; no id of " << id << endl;
            }
        }
    }

    return kSuccess;
}

// Idで指定したRecordをreturn_recordに代入する
int DataBase::readRecord(uint64_t id, Record &return_record)
{

    if (auto write_set_itr = write_set.find(id); write_set_itr != write_set.end())
    {
        // 削除済みの場合
        if (write_set_itr->second.columns.empty())
        {
            return kFailure;
        }
        else
        {
            return_record = write_set_itr->second;
        }
    }
    else
    {
        if (auto iterator = primary_index.find(id); iterator != primary_index.end())
        {
            return_record = iterator->second;
            return kSuccess;
        }
        else
        {
            cerr << FUNCNAME << "(): Error" << endl;
            return kFailure;
        }
    }
}

/*
    id : 変更する前のRecordのid
    update_record_condition : 変更後のRecord
    target_recordを使用して、tableでの添字を取得し、update_record_conditionを代入する
*/
int DataBase::updateRecord(uint64_t id, const Record &update_record_condition)
{
    // update_record_conditionが制約に反していないかチェックする
    if (checkRecord(update_record_condition, CHECK_RECORD_OPTION_UPDATE) == kFailure)
    {
        cerr << "Error: " << FUNCNAME << "(): Check Record Error" << endl;
        return kFailure;
    }

    if (auto iterator = primary_index.find(id); iterator != primary_index.end())
    {
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }
}

// updateRecord(uint64_t id, const Record &update_record_condition)のオーバーロード
int DataBase::updateRecord(const Record &target_record, const Record &update_record_condition)
{
    return updateRecord(target_record.id, update_record_condition);
}
