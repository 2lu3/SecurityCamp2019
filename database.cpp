#include "database.hpp"

#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <set>

using namespace std;
// 現在の関数名を取得
#define FUNCNAME __FUNCTION__

/* DataBase関連 */

DataBase::DataBase()
{
}

// 未実装
bool DataBase::begin()
{
    // redologの初期化
    ofstream file = ofstream("redo.log", std::ios::out);
    return kSuccess;
}

// 未実装
bool DataBase::createKey(std::string columns[])
{
    cout << columns[0] << endl;

    return kSuccess;
}

// check_recordで与えられたRecordが制約(database.hppに記載)に収まっているかチェックする
bool DataBase::checkRecord(const Record &check_record)
{
    // 制約1: idはユニークである
    if (check_record.option == Record::INSERT &&
        primary_index.find(check_record.id) != primary_index.end())
    {
        // primary_indexに登録されている
        cerr << "Error: " << FUNCNAME << "(): already registered" << endl;
        return kFailure;
    }

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

bool DataBase::saveWriteSet2RedoLog(ofstream &file)
{
    // write_set = map<id, record>
    int i = 0;

    const char SEP1 = '\x1f';
    const char SEP2 = '\x1e';

    for (const auto &[id, record] : write_set)
    {
        i++;
        stringstream out_message;
        if (record.option == Record::DELETE)
        {
            // delete
            // 完成図 : D(\x1f)D(\x1f)(\x1e)
            out_message << 'D' << SEP1 << id << SEP1 << SEP2;
        }
        else if (record.option == Record::INSERT)
        {
            // insert
            // 完成図 : I(\x1f)id(\x1f)key(\x1f)value(\x1f)...value(\x1f)(\x1e)
            // todo: 読み込むときにfor文の回数を確定させる
            out_message << "I\x1f" << id << SEP1;

            for (const auto &[column_name, column_value] : record.columns)
            {
                out_message << column_name << SEP1 << column_value << SEP1;
            }
            out_message << SEP2;
        }
        else if (record.option == Record::UPDATE)
        {
            // update
            // 完成図 : U(\x1f)id(\x1f)key(\x1f)value(\x1f)...value(\x1f)(\x1e)
            out_message << 'U' << SEP1 << id << SEP1;
            for (const auto &[column_name, column_value] : record.columns)
            {
                out_message << column_name << SEP1 << column_value << SEP1;
            }
            out_message << SEP2;
        }
        else
        {
            cerr << "Error: " << FUNCNAME << "(): no option error" << endl;
            return kFailure;
        }
        string out_message_string = out_message.str();
        uint64_t message_size = out_message_string.size();

        uint64_t hash = getHash(out_message_string);

        // todo: 直接ファイルに出力
        stringstream ss;
        ss << std::setfill('0') << std::setw(getHashDigit()) << to_string(message_size) << '\x1f' << std::setfill('0') << std::setw(getHashDigit()) << hash;
        cout << ss.str() << endl;

        file << ss.str() << SEP1 << out_message_string << flush;

        file.close();
    }
    if (file.fail())
    {
        return kFailure;
    }
    else
    {
        return kSuccess;
    }
}

bool DataBase::updateIndexOfDelete(uint64_t id)
{
    if (auto primary_index_itr = primary_index.find(id); primary_index_itr != primary_index.end())
    {
        // primary_indexに登録されていた場合

        // column_indexから消去
        // for(const auto &key_value_pair : record.columns)
        for (const auto &key_values_pair : primary_index_itr->second.columns)
        {
            // column_index == map<key_values_pair, set<id>>
            column_index[key_values_pair].erase(id);
        }

        // primary_indexからも削除
        primary_index.erase(id);
    }
    return kSuccess;
}

bool DataBase::updateIndexOfUpdate(const Record &record)
{
    auto before_column_itr = primary_index[record.id].columns.begin();
    auto after_column_itr = record.columns.begin();

    for (; after_column_itr != record.columns.end(); ++before_column_itr, ++after_column_itr)
    {
        // 前後で値が変わる場合
        if (before_column_itr->second != after_column_itr->second)
        {
            // beforeを削除
            column_index[*before_column_itr].erase(record.id);
            // afterを追加
            column_index[*after_column_itr].insert(record.id);
        }
    }

    // primary_indexの更新
    primary_index[record.id] = record;
    return kSuccess;
}

bool DataBase::updateIndexOfInsert(const Record &record)
{
    // primary_indexを更新
    primary_index[record.id] = record;

    // column_indexも更新
    for (const auto &column_name_value_pair : record.columns)
    {
        if (auto column_index_itr = column_index.find(column_name_value_pair); column_index_itr != column_index.end())
        {
            // column_indexに登録されている場合、idをsetに追加する
            column_index_itr->second.insert(record.id);
        }
        else
        {
            // column_indexに登録されていない場合、idを新しいsetとして追加する
            column_index.insert(std::make_pair(column_name_value_pair, set<uint64_t>{record.id}));
        }
    }
    return kSuccess;
}

bool DataBase::updateIndexFromWriteSet()
{
    for (const auto &[id, selected_record] : write_set)
    {
        if (selected_record.option == Record::DELETE)
        {
            updateIndexOfDelete(selected_record.id);
        }
        else if (selected_record.option == Record::UPDATE)
        {
            updateIndexOfUpdate(selected_record);
        }
        else if (selected_record.option == Record::INSERT)
        {
            updateIndexOfInsert(selected_record);
        }
        else
        {
            cerr << "Error: " << FUNCNAME << "(): invalid option" << endl;
        }
    }
    return kSuccess;
}

bool DataBase::commit()
{

    ofstream file("redo.log", std::ios::out);

    if (!file)
    {
        cerr << "Error: " << FUNCNAME << "(): can not open file";
        return kFailure;
    }

    saveWriteSet2RedoLog(file);
    // commit start
    file << "CS\x1e" << flush;
    file.close();

    return kSuccess;

    // write_setの内容をprimary_indexに反映する
    updateIndexFromWriteSet();

    // Commit Finish
    file << "CF\x1e" << flush;

    return kSuccess;
}

bool DataBase::commitTest()
{
    std::ofstream file("redo.log", std::ios::out);

    if (!file)
    {
        std::cerr << "Error " << std::endl;
        return kFailure;
    }

    saveWriteSet2RedoLog(file);
    // commit start
    file << "CS\x1e" << std::flush;
    file.close();

    cout << "recovery start " << endl;
    return crashRecovery();
}

bool DataBase::abort()
{
    // ファイルの内容を破棄する
    ofstream file("redo.log", std::ios::out);
    // write_setの初期化
    write_set.clear();
    return kSuccess;
}

bool DataBase::substituteNameValue4Record(const std::string &message, uint32_t start_pos, Record &record)
{
    uint32_t end_pos;
    // keyとvalueの読み込み
    string key, value;
    uint32_t message_size = message.size();
    do
    {
        // key
        end_pos = message.find('\x1f', start_pos + 1);
        key = message.substr(start_pos + 1, end_pos - start_pos - 1);
        start_pos = end_pos;

        // value
        end_pos = message.find('\x1f', start_pos + 1);
        value = message.substr(start_pos + 1, end_pos - start_pos - 1);
        start_pos = end_pos;
        record.columns[key] = value;
    } while (end_pos < message_size - 2);
    return kSuccess;
}

uint32_t DataBase::substituteID4Record(const string &message, Record &record)
{
    uint32_t start_pos = 2;
    uint32_t end_pos = message.find('\x1f', start_pos + 1);
    cout << message.substr(start_pos + 1, end_pos - start_pos - 1) << endl;
    uint64_t id = stoull(message.substr(start_pos + 1, end_pos - start_pos - 1));
    record.id = id;
    return end_pos;
}

bool DataBase::writeInsert2WriteSetFromRedoLog(const string &message)
{
    Record record;
    record.option = Record::INSERT;
    uint32_t position;
    // IDをrecordにセット
    position = substituteID4Record(message, record);
    // NameとValueをrecordにセット
    substituteNameValue4Record(message, position, record);

    if (checkRecord(record) == kFailure)
    {
        return kFailure;
    }

    write_set.insert(std::make_pair(record.id, record));
    return kSuccess;
}

bool DataBase::writeUpdate2WriteSetFromRedoLog(const string &message)
{
    Record record;
    record.option = Record::UPDATE;
    uint32_t position;
    // IDをrecordにセット
    position = substituteID4Record(message, record);
    // NameとValueをrecordにセット
    substituteNameValue4Record(message, position, record);

    // primary_indexに追加済みの場合
    if (checkRecord(record) == kFailure)
    {
        return kFailure;
    }
    return kSuccess;
}

bool DataBase::writeDelete2WriteSetFromRedoLog(const std::string &message)
{
    Record record;
    record.option = Record::DELETE;
    substituteID4Record(message, record);
    write_set[record.id] = record;
    return kSuccess;
}

bool DataBase::createWriteSetWithCheck(stringstream &sstream)
{
    /* sstreamの完成図 */
    /*
    CS(\x1e)
    命令文
    I(\x1f)id(\x1f)key(\x1f)value(\x1f)key(...)value(\x1f)(\x1e)(実際には改行文字なし)
    U(\x1f)id(\x1f)key(\x1f)value(\x1f)key(...)value(\x1f)(\x1e)(実際には改行文字なし)
    D(\x1f)id(\x1f)(実際には改行文字なし)
    の繰り返し
    CF(\x1e)
    */
    // redo.log書き込み未終了 : -1
    // redo.log書き込み終了 : 0
    // commit処理終了 : 1
    int is_redolog_output_finished = -1;
    string all_message = sstream.str();
    uint64_t all_message_size = all_message.size();
    string message;
    // redo.logの命令文単位での区切り文字は、0x1e
    uint64_t now_reading_position = 0;
    while (now_reading_position < all_message_size - 3)
    {
        uint64_t data_digit = stoull(all_message.substr(now_reading_position, getHashDigit()));
        uint64_t hash = stoull(all_message.substr(now_reading_position + getHashDigit() + 1, getHashDigit()));
        uint64_t sentence_digit = data_digit + getHashDigit() * 2 + 1;
        message = all_message.substr(now_reading_position + getHashDigit() * 2 + 2, data_digit);

        if (getHash(message) != hash)
        {
            cerr << "Error: " << FUNCNAME << "(): hash error " << getHash(message) << " " << hash << endl;
            return kFailure;
        }
        if (message.substr(0, 2) == "CS")
        {
            // 上の完成図参照
            // CSがないredo.logは壊れている
            is_redolog_output_finished = 0;
        }
        else if (message.substr(0, 2) == "CF")
        {
            // commit処理 終了
            is_redolog_output_finished = 1;
        }
        else if (message[0] == 'D')
        {
            writeDelete2WriteSetFromRedoLog(message);
        }
        else if (message[0] == 'I')
        {
            writeInsert2WriteSetFromRedoLog(message);
        }
        else if (message[0] == 'U')
        {
            writeUpdate2WriteSetFromRedoLog(message);
        }
        else
        {
            cerr << "undefined command " << message << endl;
            return kFailure;
        }

        now_reading_position += sentence_digit + 1;
    }

    if (is_redolog_output_finished == -1)
    {
        // redo.logへの書き込みがまだ終わっていない -> recoveryしない
        return kFailure;
    }
    else if (is_redolog_output_finished == 0)
    {
        // redo.logへの書き込みは終わっている
        return kSuccess;
    }
    else
    {
        // commit処理も終わっている
        write_set.clear();
        return kSuccess;
    }
}

// 未実装
bool DataBase::crashRecovery()
{
    // write_setの初期化
    write_set.clear();

    ifstream file("redo.log", std::ios::in);
    if (!file)
    {
        cerr << "Error: " << FUNCNAME << "(): Failed to load file" << endl;
        return kFailure;
    }
    // string
    stringstream sstream;
    sstream << file.rdbuf();

    file.close();

    // redo.logの操作が不正でないかのチェックと、redo.logの内容をwrite_setに変換する
    if (createWriteSetWithCheck(sstream) == kFailure)
    {
        return kFailure;
    }

    cout << "write set " << endl;
    for (auto &[id, record] : write_set)
    {
        cout << "id " << id << " name " << record.columns["name"] << endl;
    }

    return commit();
}

/* DataBase Delete, Insert, Read, Update */

// deleteRecord(uint64_t id)のオーバーロード
bool DataBase::deleteRecord(const Record &target_record)
{
    return deleteRecord(target_record.id);
}

// target_recordで指定したRecordを削除する
bool DataBase::deleteRecord(uint64_t id)
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

bool DataBase::setID2Record(Record &target_record)
{
    // primary_indexにもwrite_setにもないidをランダムで生成
    uint64_t id = randID();

    auto primary_itr = primary_index.find(id);
    auto write_set_itr = write_set.find(id);

    while (primary_itr != primary_index.end() || write_set_itr != write_set.end() || id == Record::kIdNull)
    {
        id = randID();
        primary_itr = primary_index.find(id);
        write_set_itr = write_set.find(id);
    }

    // idをセット
    target_record.id = id;
    return kSuccess;
}

// table[]の末尾に新しいRecordを追加
bool DataBase::insertRecord(Record &new_record)
{
    new_record.option = Record::INSERT;
    setID2Record(new_record);
    if (checkRecord(new_record) == kFailure)
    {
        cerr << "Error: " << FUNCNAME << "(): checkRecord() error" << endl;
        return kFailure;
    }
    write_set[new_record.id] = new_record;

    return kSuccess;
}

bool DataBase::searchInWriteSet(const std::pair<std::string, std::string> column_name_value_pair, std::set<std::uint64_t> &column_sets)
{
    // write set検索
    // write set(map型)内のRecordをすべて回す
    // write_set_itr->second = write_set内のRecord
    for (auto write_set_itr = write_set.begin(); write_set_itr != write_set.end(); ++write_set_itr)
    {
        // Recordのcolumnsのcolumn_nameがcolumn_valueかどうか
        // itrは、columnsのiterator
        // if(auto itr = record.columns.find(column_name); itr != record.columns.end())
        auto &selected_record = write_set_itr->second;
        if (auto itr = selected_record.columns.find(column_name_value_pair.first); itr != selected_record.columns.end())
        {
            // Record.column[column_name] == value
            if (itr->second == column_name_value_pair.second)
            {
                // column_sets[iが0なら0、それ以外なら1]に追加
                column_sets.insert(selected_record.id);
            }
        }
    }
    return kSuccess;
}

bool DataBase::searchInDB(const std::pair<std::string, std::string> column_name_value_pair, std::set<std::uint64_t> &column_sets)
{
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
                column_sets.insert(id);
            }
        }
    }
    else
    {
        if (auto column_names_itr = column_names.find(column_name_value_pair.first); column_names_itr != column_names.end())
        {
            //  keyが正しい場合は、パス
            return kSuccess;
        }
        else
        {
            cerr << "Error: " << FUNCNAME << "(): column_name and value error" << endl;
            return kFailure;
        }
    }
    return kSuccess;
}

bool DataBase::mergeBeforeAfter(std::set<uint64_t> &base_set, std::set<uint64_t> &new_set)
{
    auto itr0 = base_set.begin();
    auto itr1 = new_set.begin();
    while (itr0 != base_set.end() && itr1 != new_set.end())
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
            itr0 = base_set.erase(itr0);
        }
        else
        {
            // *itr0 > *itr1
            // 小さいほうは、==になることはないので飛ばす
            // 保存するのは、itr0のほうだけ(消すのはもったないない)
            ++itr1;
        }
    }
    return kSuccess;
}

/*
    columnsで指定した条件にあうRecord(複数可)を返す関数
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
bool DataBase::readRecord(Columns columns, vector<Record> &return_records)
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

    // 2つsetを用意して、mergeする
    // 0: 今まで絞り込んだset
    // 1: 今から絞り込むset
    set<uint64_t> column_sets[2];
    int i = 0;

    // 条件を順番に調べていくぅ
    for (const auto &column_name_value_pair : columns)
    {
        if (auto column_names_itr = column_names.find(column_name_value_pair.first); column_names_itr == column_names.end())
        {
            // column_nameが、column_namesに登録されていない場合
            cerr << "Error " << FUNCNAME << "(): column name error" << endl;
            return kFailure;
        }

        searchInWriteSet(column_name_value_pair, column_sets[i == 0]);
        if (i == 0)
        {
            continue;
        }
        searchInDB(column_name_value_pair, column_sets[i == 0]);
        mergeBeforeAfter(column_sets[0], column_sets[1]);
        ++i;
    }

    // 結果をreturn_recordに代入する
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
bool DataBase::readRecord(uint64_t id, Record &return_record)
{

    if (auto write_set_itr = write_set.find(id); write_set_itr != write_set.end())
    {
        if (write_set_itr->second.option == Record::DELETE)
        {
            // 削除済みの場合
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
    return kSuccess;
}

/*
    id : 変更する前のRecordのid
    update_record_condition : 変更後のRecord
    target_recordを使用して、tableでの添字を取得し、update_record_conditionを代入する
*/
bool DataBase::updateRecord(uint64_t id, Record &update_record_condition)
{
    update_record_condition.option = Record::UPDATE;
    // update_record_conditionが制約に反していないかチェックする
    if (checkRecord(update_record_condition) == kFailure)
    {
        cerr << "Error: " << FUNCNAME << "(): Check Record Error" << endl;
        return kFailure;
    }

    if (auto iterator = primary_index.find(id); iterator != primary_index.end())
    {
        if (auto itr = write_set.find(id); itr != primary_index.end())
        {
            // write_setにすでにある場合
            if (itr->second.option == Record::DELETE)
            {
                // 削除されたRecordの場合
                cerr << "Error: " << FUNCNAME << "(): alredy deleted" << endl;
                return kFailure;
            }
            else
            {
                // 削除されていない場合
                itr->second = update_record_condition;
            }
        }
        else
        {
            // write_setにまだ登録されていない場合
            itr->second = update_record_condition;
        }
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }
    return kSuccess;
}

// updateRecord(uint64_t id, const Record &update_record_condition)のオーバーロード
bool DataBase::updateRecord(const Record &target_record, Record &update_record_condition)
{
    return updateRecord(target_record.id, update_record_condition);
}
