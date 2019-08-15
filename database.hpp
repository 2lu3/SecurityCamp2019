#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <utility>
#include <functional>

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
    using Columns = std::map<std::string, std::string>;
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
        const static std::uint64_t kIdNull = 0;
        std::uint64_t id;
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
    using PriamryIndex = std::map<std::uint64_t, Record>;

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
    bool updateRecord(std::uint64_t id, const Record &update_record_condition);
    // target_recordで指定したRecordをupdate_record_conditionに置き換える
    bool updateRecord(const Record &target_record, const Record &update_record_condition);

    // target_recordで指定したRecordを消去する
    bool deleteRecord(const Record &target_record);
    // idで指定したRecordを消去する
    bool deleteRecord(std::uint64_t id);

    // target_columnsで指定した条件に合うRecordを返す
    bool readRecord(Columns columns, std::vector<Record> &return_records);
    // idで指定したRecordを返す
    bool readRecord(std::uint64_t id, Record &return_record);

    // new_recordのコピーをtableに追加する
    bool insertRecord(Record &new_record);

    // 現在の、tableに格納されているRecordの数
    std::uint32_t table_num = 0;

    std::map<std::uint64_t, Record> primary_index;

    std::map<std::pair<std::string, std::string>, std::set<std::uint64_t>> column_index;

    bool commitTest();

    class RandID
    {
    public:
        RandID() : mt(std::random_device()()), dist(1, UINT64_MAX)
        {
        }

        inline std::uint64_t operator()()
        {
            return dist(mt);
        }

    private:
        std::mt19937 mt;
        std::uniform_int_distribution<uint64_t> dist;
    };
    RandID randID;

    // Recordが制約に収まっているかチェックする
    bool
    checkRecord(const Record &check_record);
    bool setID2Record(Record &target_record);

    /* commit 関連の補助関数 */
    bool saveWriteSet2RedoLog(std::ofstream &file);
    bool updateIndexFromWriteSet();
    bool updateIndexOfDelete(std::uint64_t id);     // updateIndexFromSet専用
    bool updateIndexOfUpdate(const Record &record); // updateIndexFromSet専用
    bool updateIndexOfInsert(const Record &record); // updateIndexFromSet専用

    /* crash recovery 関連の補助関数*/
    // redo.logを格納したsstreamを読み込み、write_setに変換する
    bool createWriteSetWithCheck(std::stringstream &sstream);
    // createWriteSetWithCheck専用
    bool substituteNameValue4Record(const std::string &message, std::uint32_t start_pos, Record &record);
    // createWriteSetWithCheck専用
    std::uint32_t substituteID4Record(const std::string &message, Record &record);
    // createWriteSetWithCheck専用
    bool writeInsert2WriteSetFromRedoLog(const std::string &message);
    // createWriteSetWithCheck専用
    bool writeUpdate2WriteSetFromRedoLog(const std::string &message);
    bool writeDelete2WriteSetFromRedoLog(const std::string &message);

    std::set<std::string> column_names = {"name", "age"};

    // id-tableの添字を格納する
    // update : write_setに書き換えたものを代入
    // insert : primary_indexにないidを追加
    // delete : map.size() = 0
    std::map<std::uint64_t, Record> write_set;
    // map<pair<column名, value>, set<id>> : keyを指定したうえで、valueからidを効率的に探す
private:
};
