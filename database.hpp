#ifndef SECURITYCAMP2019_DATABASE_HPP_
#define SECURITYCAMP2019_DATABASE_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>

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

class RedoLog;

class DataBase
{
public:
    DataBase();

    /*  制約
        1: idはユニークである
        2: idは0以外である
        3: columnsのkeyはすべてのRecordで共通でなければならない */
    struct Record
    {
        const static std::uint64_t kIdNull = 0;
        const static std::string kValueNull;

        std::uint64_t id;
        std::map<std::string, std::string> columns;
        Record();
    };
    const static int table_max_num = 100; // 保持できるRecordの最大値(デバッグ用にpublicにおいているが、privateに移す予定)
    Record table[table_max_num];          // データベースのデータそのもの(デバッグ用にpublicにおいているが、privateに移す予定)

    // 関数の戻り値で、成功を表す
    const static int kSuccess = 1;
    // 関数の戻り値で、失敗を表す
    const static int kFailure = 0;

    int crashRecovery();                  // 未実装
    int begin();                          // 未実装
    int commit();                         // 未実装
    int abort();                          // 未実装
    int createKey(std::string columns[]); // 未実装

    // idで指定したRecordをupdate_record_conditionに置き換える
    int updateRecord(std::uint64_t id, const Record &update_record_condition);
    // target_recordで指定したRecordをupdate_record_conditionに置き換える
    int updateRecord(const Record &target_record, const Record &update_record_condition);

    // target_recordで指定したRecordを消去する
    int deleteRecord(const Record &target_record);
    // idで指定したRecordを消去する
    int deleteRecord(std::uint64_t id);

    // target_columnsで指定した条件に合うRecordを返す
    int readRecord(const std::map<std::string, std::string> &target_columns, std::vector<Record> &return_records);
    // idで指定したRecordを返す
    int readRecord(std::uint64_t id, Record &return_record);

    // new_recordのコピーをtableに追加する
    int insertRecord(Record &new_record);

    // target_recordにまだ登録されていないIDを設定する
    int setId2Record(Record &target_record);

    std::unique_ptr<RedoLog> redoLog;

private:
    // 現在の、tableに格納されているRecordの数
    int table_num = 0;

    std::random_device rnd;

    const int CHECK_RECORD_OPTION_INSERT = 0;
    const int CHECK_RECORD_OPTION_UPDATE = 1;
    // Recordが制約に収まっているかチェックする
    int checkRecord(const Record &check_record, int option);

    std::set<std::string> column_names{"name", "age"};

    // id-tableの添字を格納する
    std::map<std::uint64_t, std::uint32_t> primary_index;
};

class UndoLog
{
public:
private:
};

class RedoLog
{
public:
    // Insertを記録する
    int addInsertLog(const DataBase::Record &record);
    // Updateの前後の差分を記録する
    int addUpdateLog(const DataBase::Record &before_record, const DataBase::Record &updated_record);
    // Deleteを記録する
    int addDeleteLog(std::uint64_t id);

    int readRedoLog(std::string &message);

    const static int kSuccess = DataBase::kSuccess; // 成功を示す関数の戻り値
    const static int kFaliure = DataBase::kFailure; // 失敗を示す関数の戻り値

private:
    const char log_file_name[10] = "redo.log";
};

#endif //SECURITYCAMP2019_DATABASE_HPP_
