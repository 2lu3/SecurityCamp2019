#include "database.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::extent;
using std::ifstream;
using std::int64_t;
using std::map;
using std::ofstream;
using std::string;
using std::to_string;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::vector;

// 現在の関数名を取得
#define FUNCNAME __FUNCTION__

/* RedoLog関連 */

int RedoLog::readRedoLog(string &message)
{
    ifstream file(log_file_name);
    if (!file)
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }

    // ファイルの内容を一括読み込み
    // ソース: https://fa11enprince.hatenablog.com/entry/2014/04/03/233500
    std::istreambuf_iterator<char> iterator(file);
    std::istreambuf_iterator<char> last;

    // message(iterator, last)としたかったが、エラーが出てきたので諦めた
    string buffer(iterator, last);
    message = buffer;

    // 4 debug
    cout << message << endl;
    return kSuccess;
}

/* RedoLog Delete Insert Update */

// DeleteのRedoLogをredo.logに記録する
int RedoLog::addDeleteLog(uint64_t id)
{
    // 完成図 : DELETE(\x1f)id(\x1e)
    string log_message = "DELETE\x1f" + to_string(id) + '\x1e';

    // ファイルへの書き込み
    ofstream file(log_file_name, std::ios::app);
    if (file)
    {
        file << log_message << endl;
        if (!file)
        {
            cerr << FUNCNAME << "(): Error" << endl;
            return kFaliure;
        }
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }
    return kSuccess;
}

// InsertのRedoLogをredo.logに記録する
int RedoLog::addInsertLog(const DataBase::Record &record)
{
    // 完成図 : INSERT(\x1f)key(\x1f)value(\x1f)...(\x1f)key(\x1f)value(\x1e)
    string log_message = "INSERT";

    for (const auto &[key, value] : record.columns)
    {
        log_message += '\x1f' + key + '\x1f' + value;
    }
    log_message += '\x1e';

    // ファイルへの書き込み
    ofstream file(log_file_name, std::ios::app);
    if (file)
    {
        file << log_message << endl;
        if (!file)
        {
            cerr << FUNCNAME << "(): Error" << endl;
            return kFaliure;
        }
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }

    return kSuccess;
}

// UpdateのRedoLogをredo.logに記録する
int RedoLog::addUpdateLog(const DataBase::Record &before_record, const DataBase::Record &updated_record)
{
    // 完成図 : UPDATE(\x1f)id(\x1f)変更するkey(\x1f)変更するvalue(\x1f)...(\x1f)変更するkey(\x1f)変更するvalue(\x1e)
    // RedoLogの内容
    string log_message = "UPDATE\x1f" + to_string(updated_record.id);

    // before_recordとupdate_recordのcolumnのvalueを順番に比較する
    auto before_record_iterator = before_record.columns.begin();
    auto updated_record_iterator = updated_record.columns.begin();
    for (;
         before_record_iterator != before_record.columns.end();
         ++before_record_iterator, ++updated_record_iterator)
    {
        // keyが同じであることは、updateRecordで確認済み
        if (before_record_iterator->second != updated_record_iterator->second)
        {
            // valueが一致しない場合、変更されている
            // log_message += (\x1f)key(\x1f)value
            log_message += '\x1f' + updated_record_iterator->first + '\x1f' + updated_record_iterator->second;
        }
    }
    log_message += '\x1e';

    // ファイルへの書き込み
    ofstream file(log_file_name, std::ios::app);
    if (file)
    {
        file << log_message << endl;
        if (!file)
        {
            cerr << FUNCNAME << "(): Error" << endl;
            return kFaliure;
        }
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }

    return kSuccess;
}

/* Record関連 */

DataBase::Record::Record()
{
    // idはnull値で初期化
    id = kIdNull;
}

/* DataBase関連 */

DataBase::DataBase() : redoLog(new RedoLog)
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
    cout << columns[0] << endl;
    return kSuccess;
}

// target_record.idにまだ登録されていないIDを代入する
int DataBase::setId2Record(Record &target_record)
{
    // idがnullか、すでにidが登録されている場合は、idを変更する必要がある
    if (target_record.id == Record::kIdNull || primary_index.count(target_record.id) == 1)
    {
        uint64_t random_id = rnd();
        // while(idが登録されている || id==null(=0))
        while (primary_index.count(random_id) == 1 || random_id == Record::kIdNull)
        {
            // random_idを更新
            random_id = rnd();
        }
        // まだ登録されていないidを代入
        target_record.id = random_id;
    }
    return kSuccess;
}

// check_recordで与えられたRecordが制約(database.hppに記載)に収まっているかチェックする
// option: チェックする内容がRecordのinsertとupdateで微妙に違う 0:insert 1:update
int DataBase::checkRecord(const Record &check_record, int option)
{
    // 制約1: idはユニークである
    // insertのときのみ
    // 追記 : RedoLogの実装で必要なくなった
    // if (option == CHECK_RECORD_OPTION_INSERT)
    // {
    //     // idがすでに登録されている場合
    //     if (primary_index.count(check_record.id) != 0)
    //     {
    //         return kFailure;
    //     }
    // }

    // 制約2: idは0以外である
    // Insertのときは後でIDを振るのでパス
    if (option != CHECK_RECORD_OPTION_INSERT)
    {
        if (check_record.id == Record::kIdNull)
        {
            return kFailure;
        }
    }

    // 制約3: columnに登録されている(map型での)keyの集合は、column_namesに格納されている文字列の集合と等価でなければならない
    // insert, update 両方

    // 最初に要素数を比べる
    if (column_names.size() != check_record.columns.size())
    {
        // 要素数が違う場合、等価でない
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
            return kFailure;
        }
    }

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

/* DataBase Delete, Insert, Read, Update */

// deleteRecord(uint64_t id)のオーバーロード
int DataBase::deleteRecord(const Record &target_record)
{
    return deleteRecord(target_record.id);
}

// target_recordで指定したRecordを削除する
int DataBase::deleteRecord(uint64_t id)
{
    if (id == Record::kIdNull)
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }

    // target_recordのidからprimary_indexを使用して
    uint32_t target_table_index;
    if (auto iterator = primary_index.find(id); iterator != end(primary_index))
    {
        return redoLog->addDeleteLog(id);
        // target_table_index = iterator->second;
    }
    else
    {
        // そのidのrecordは存在しない
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }
    // for (int i = target_table_index; i < table_num - 1; ++i)
    // {
    //     table[i] = table[i + 1];
    // }
    // --table_num;
    // primary_index.erase(target_table_index);
    return kSuccess;
}

// table[]の末尾に新しいRecordを追加
// idを割り振るため、const Record &new_recordにしていない
int DataBase::insertRecord(Record &new_record)
{
    // IDをセット
    // setId2Record(new_record);
    // Recordの制約チェック
    if (checkRecord(new_record, CHECK_RECORD_OPTION_INSERT) == kFailure)
    {
        cerr << FUNCNAME << "(): Failed to insert Record" << endl;
        return kFailure;
    }
    return redoLog->addInsertLog(new_record);
    // table[table_num] = new_record;
    // primary_index[new_record.id] = table_num;
    // table_num++;
    // return kSuccess;
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
    for (int i = 0; i < table_num; ++i)
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

// Idで指定したRecordをreturn_recordに代入する
int DataBase::readRecord(uint64_t id, Record &return_record)
{
    if (id == Record::kIdNull)
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }

    if (auto iterator = primary_index.find(id); iterator != primary_index.end())
    {
        return_record = table[iterator->second];
        return kSuccess;
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }
}

/*
    id : 変更する前のRecordのid
    update_record_condition : 変更後のRecord
    target_recordを使用して、tableでの添字を取得し、update_record_conditionを代入する
*/
int DataBase::updateRecord(uint64_t id, const Record &update_record_condition)
{
    // idが変更前と変更後で一致するかチェック
    if (id != update_record_condition.id)
    {
        cerr << FUNCNAME << "(): id of target_record and update_record_condition is not same" << endl;
        return kFailure;
    }

    // update_record_conditionが制約に反していないかチェックする
    if (checkRecord(update_record_condition, CHECK_RECORD_OPTION_UPDATE) == kFailure)
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }

    // idから、primary_indexを使って、table[]の添字を取得
    if (auto iterator = primary_index.find(id); iterator != primary_index.end())
    {
        // 変更する対象の添字に、update_record_conditionを代入
        return redoLog->addUpdateLog(table[iterator->second], update_record_condition);
        // table[target_table_index_iterator->second] = update_record_condition;
        // return kSuccess;
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
