#include <iostream>
#include <map>
#include <string>
#include <map>

#include "database.hpp"

using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

int main()
{
    system("chcp 65001");
    DataBase dataBase;
    DataBase::Record record1, record2;

    // Recordの追加(x 2)
    record1.columns["name"] = "山田";
    record1.columns["age"] = "12";
    dataBase.insertRecord(record1);

    record2.columns["name"] = "田中";
    record2.columns["age"] = "24";
    dataBase.insertRecord(record2);

    dataBase.commit();

    vector<DataBase::Record> vec;
    dataBase.readRecord(record1.columns, vec);
    dataBase.deleteRecord(vec[0]);

    dataBase.commit();
    std::string message;
    // dataBase.redoLog->readRedoLog(message);

    // 保存されているかの確認
    // cout << dataBase.table[0].id << " " << dataBase.table[0].columns["name"] << dataBase.table[0].columns["age"] << endl;

    for (uint32_t i = 0; i < dataBase.table_num; ++i)
    {
        cout << i << " " << dataBase.table[i].id << " " << dataBase.table[i].columns["name"] << endl;
    }

    // vector<DataBase::Record> vec;   // 条件に合うRecordを格納するための変数
    // map<string, string> conditions; // 条件
    // conditions["name"] = "田中";    // 条件1(nameが田中であるという条件)
    // dataBase.readRecord(conditions, vec);

    // for (auto &value : vec)
    // {
    //     // 条件に合うRecordのageを確認
    //     cout << value.columns["age"] << endl;
    // }
}
