#ifndef SECURITYCAMP2019_DATABASE_HPP_
#define SECURITYCAMP2019_DATABASE_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>

class RedoLog;

class DataBase
{
public:
    /*  制約
        1: idはユニークである
        2: idは0以外である
        3: columnsのkeyはすべてのRecordで共通でなければならない
     */
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
    const static int kSuccess = 1;        // 関数の戻り値で、成功を表す
    const static int kFailure = 0;        // 関数の戻り値で、失敗を表す
    DataBase();
    int crashRecovery();                                                                  // 未実装
    int begin();                                                                          // 未実装
    int updateRecord(std::uint64_t id, const Record &update_record_condition);            // idで指定したRecordをupdate_record_conditionに置き換える
    int updateRecord(const Record &target_record, const Record &update_record_condition); // target_recordで指定したRecordをupdate_record_conditionに置き換える
    int deleteRecord(const Record &target_record);                                        // target_recordで指定したRecordを消去する
    int deleteRecord(std::uint64_t id);                                                   // idで指定したRecordを消去する
    int commit();                                                                         // 未実装
    int abort();                                                                          // 未実装
    int createKey(std::string columns[]);                                                 // 未実装

    int readRecord(const std::map<std::string, std::string> &target_columns, std::vector<Record> &return_records); // target_columnsで指定した条件に合うRecordを返す
    int readRecord(std::uint64_t id, Record &return_record);                                                       // idで指定したRecordを返す
    int insertRecord(Record &new_record);                                                                          // new_recordのコピーをtableに追加する
    int setId2Record(Record &target_record);                                                                       // target_recordにまだ登録されていないIDを設定する(この関数を作らないと、他の関数の引数にconstをつけることができない)

private:
    std::unique_ptr<RedoLog> redoLog;

    // 現在の、tableに格納されているRecordの数
    int table_num = 0;

    std::random_device rnd;

    const int CHECK_RECORD_OPTION_INSERT = 0;
    const int CHECK_RECORD_OPTION_UPDATE = 1;
    int checkRecord(const Record &check_record, int option); // Recordが制約に収まっているかチェックする

    std::set<std::string> column_names{"name", "age"};
    std::map<std::uint64_t, std::uint32_t> primary_index; // id-tableの添字を格納する
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
    int addInsertLog(std::uint32_t table_index, const DataBase::Record &record);
    // Updateの前後の差分を記録する (before_record:変更前, updated_record:変更後)
    int addUpdateLog(std::uint32_t table_index, const DataBase::Record &before_record, const DataBase::Record &updated_record);
    // Deleteを記録する
    int addDeleteLog(std::uint64_t id);

    const int kSuccess = DataBase::kSuccess; // 成功を示す関数の戻り値
    const int kFaliure = DataBase::kFailure; // 失敗を示す関数の戻り値

private:
    const char log_file_name[10] = "redo.log";
};

#endif //SECURITYCAMP2019_DATABASE_HPP_
