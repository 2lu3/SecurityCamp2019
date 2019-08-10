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
    system("chcp 932");
    DataBase dataBase;
    DataBase::Record record1, record2;

    // Recordの追加(x 2)
    record1.columns["name"] = "山田";
    record1.columns["age"] = "12";
    dataBase.insertRecord(record1);

    record2.columns["name"] = "田中";
    record2.columns["age"] = "24";
    dataBase.insertRecord(record2);

    record2.id = record1.id;
    dataBase.updateRecord(record1, record2);
    dataBase.deleteRecord(record1);

    // 保存されているかの確認
    // cout << dataBase.table[0].id << " " << dataBase.table[0].columns["name"] << dataBase.table[0].columns["age"] << endl;

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
