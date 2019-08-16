#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <utility>
#include <functional>

#include "FileIo.hpp"

/*  データベース設計
1. Read, Insert, Update, Deleteを複数回実行
    * ここでの操作は、Commitするまでデータベースには反映されない
    * 操作は、redo.logに記録される
    * Read
        * Commit済みのデータしかReadできない
    * Insert
        * 実際にデータが追加されるのは、Commitしたとき
    * Update
        * Commit済みのデータしかUpdateできない
    * Delete
        * Commit済みのデータしかDeleteできない
2. Commit または Abort を実行する
    * Commit
        * redo.logを読み込み、順番に実行する
        * すべて実行した後、redo.logの内容を削除する
    * Abort
        * redo.logを破棄する
 */

class DataBase
{
public:
    // column_name, column value
    using Id = std::uint64_t;
    using ColumnName = std::string;
    using ColumnValue = std::string;
    using Column = std::pair<ColumnName, ColumnValue>;
    using Columns = std::map<ColumnName, ColumnValue>;
    using ColumnIndexes = std::map<Column, std::set<Id>>;
    DataBase();

    /*  制約
        1: idはユニークである
        2: idはkIdNull以外である
        3: columnsのkeyはすべてのRecordで共通でなければならない */
    struct Record
    {
        Record()
        {
            id = kIdNull;
            option = NO_OPTION;
        }
        Record(uint64_t new_id)
        {
            id = new_id;
        }
        const static Id kIdNull = 0;
        Id id;
        Columns columns;

        enum OperateOption
        {
            NO_OPTION,
            INSERT,
            UPDATE,
            DELETE
        };
        OperateOption option;
    };
    using PriamryIndex = std::map<Id, Record>;

    // 関数の戻り値で、成功を表す
    const static bool kSuccess = true;
    // 関数の戻り値で、失敗を表す
    const static bool kFailure = false;

    bool crashRecovery();                  // 未実装
    bool begin();                          // 未実装
    bool commit();                         // 未実装
    bool abort();                          // 未実装
    bool createKey(std::string columns[]); // 未実装

    // idで指定したRecordをupdate_record_conditionに置き換える
    bool updateRecord(Id id, Record &update_record_condition);
    // target_recordで指定したRecordをupdate_record_conditionに置き換える
    bool updateRecord(const Record &target_record, Record &update_record_condition);

    // target_recordで指定したRecordを消去する
    bool deleteRecord(const Record &target_record);
    // idで指定したRecordを消去する
    bool deleteRecord(Id id);

    // target_columnsで指定した条件に合うRecordを返す
    bool readRecord(Columns columns, std::vector<Record> &return_records);
    // idで指定したRecordを返す
    bool readRecord(Id id, Record &return_record);

    // new_recordのコピーをtableに追加する
    bool insertRecord(Record &new_record);

    // 現在の、tableに格納されているRecordの数
    std::uint32_t table_num = 0;

    std::map<Id, Record> primary_index;

    ColumnIndexes column_index;

    bool commitTest();

    class RandID
    {
    public:
        RandID() : mt(std::random_device()()), dist(1, UINT64_MAX)
        {
        }

        inline Id operator()()
        {
            return dist(mt);
        }

    private:
        std::mt19937 mt;
        std::uniform_int_distribution<Id> dist;
    };
    RandID randID;

    // Recordが制約に収まっているかチェックする
    bool
    checkRecord(const Record &check_record);
    Id generateId();
    bool setID2Record(Record &target_record);

    /* commit 関連の補助関数 */
    bool saveWriteSet2RedoLog(std::ofstream &file);
    bool updateIndexFromWriteSet();
    bool updateIndexOfDelete(Id id);                // updateIndexFromSet専用
    bool updateIndexOfUpdate(const Record &record); // updateIndexFromSet専用
    bool updateIndexOfInsert(const Record &record); // updateIndexFromSet専用
    bool writeCommitStart2RedoLog(std::ofstream &file);
    bool writeCommitFinish2RedoLog(std::ofstream &file);

    /* crash recovery 関連の補助関数*/
    // redo.logを格納したsstreamを読み込み、write_setに変換する
    bool createWriteSetWithCheck(std::stringstream &sstream);
    // createWriteSetWithCheck専用
    bool deserializeNameValue4Record(const std::string &message, std::uint32_t start_pos, Record &record);
    // createWriteSetWithCheck専用
    std::uint32_t deserializeID4Record(const std::string &message, Record &record);
    // createWriteSetWithCheck専用
    bool writeInsert2WriteSetFromRedoLog(const std::string &message);
    // createWriteSetWithCheck専用
    bool writeUpdate2WriteSetFromRedoLog(const std::string &message);
    bool writeDelete2WriteSetFromRedoLog(const std::string &message);

    /* read record 関連の補助関数 */
    bool searchInWriteSet(const Column column_name_value_pair, std::set<Id> &column_sets);
    bool searchInDB(const Column column_name_value_pair, std::set<Id> &column_sets);
    bool mergeBeforeAfter(std::set<Id> &base_set, std::set<Id> &new_set);
    Record *searchRecordInWriteSetById(Id id);

    std::set<std::string> column_names = {"name", "age"};

    std::map<Id, Record> write_set;

private:
    uint32_t getHashDigit()
    {
        const static uint32_t digit = std::to_string(UINT64_MAX).size();
        return digit;
    }

    uint64_t getHash(std::string &str)
    {
        static std::hash<std::string> hash_fn;
        return hash_fn(str);
    }

    FileIo fileIo;
    const std::string file_name = "redo.log";
};
