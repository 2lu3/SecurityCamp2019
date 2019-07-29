#ifndef SECURITYCAMP2019_DATABASE_HPP_
#define SECURITYCAMP2019_DATABASE_HPP_

#include <cstdint>
#include <string>
#include <iostream>
#include <map>
#include <random>
#include <set>

class DataBase
{
public:
    struct Record
    {
        // column
        // map<string, string>型を使うことで、自由かつ動的にcolumnを扱えるようにする
        // 用例:
        // columns["name"] = "Hoge";
        // columns["age"] = "2020";
        std::map<std::string, std::string> columns;
    };
    const static int table_max_num = 100; // 保持できるRecordの最大値(デバッグ用にpublicにおいているが、privateに移す予定)
    Record table[table_max_num];          // データベースのデータそのもの(デバッグ用にpublicにおいているが、privateに移す予定)
    const static int kSuccess = 1;        // 関数の戻り値で、成功を表す
    const static int kFailure = 0;        // 関数の戻り値で、失敗を表す
    DataBase();
    int crashRecovery();                  // 未実装
    int begin();                          // 未実装
    int updateRecord();                   // 未実装
    int deleteRecord();                   // 未実装
    int commit();                         // 未実装
    int abort();                          // 未実装
    int createKey(std::string columns[]); // 未実装

    int readRecord(const std::map<std::string, std::string> &target_columns, std::vector<Record> &return_records); // target_columnsで指定した条件に合うRecordを返す
    int insertRecord(Record new_record);                                                                           // new_recordのコピーをtableに追加する

private:
    // 現在の、tableに格納されているRecordの数
    int table_num = 0;

    std::set<std::string> column_names{"name", "age"};
};

#endif //SECURITYCAMP2019_DATABASE_HPP_
