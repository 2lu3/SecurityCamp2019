#include <iostream>
#include <map>
#include <string>
#include <map>
#include <fstream>

#include "database.hpp"
#include "FileIo.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::string;
using std::vector;

/*
1. read, update, delete, insert

2. commit開始

エラー：commit失敗扱い -> abort処理へ
クラッシュ：なにも作業していないので、crashRecoveryでcommitするだけ

2-1. commit開始をredo.logに追加

エラー：commit失敗扱い -> abort処理へ
クラッシュ：commit処理を実行

3. commit処理中

エラー：commit

4. commit終了
2-2. redo.logを消去

*/

int main()
{
    system("chcp 65001");
    DataBase dataBase;

    DataBase::Record record[50];

    // データの入力
    std::ifstream file("data.csv");
    if (!file)
    {
        cerr << "Failed to load data.csv" << endl;
        return 1;
    }
    string buffer;
    int i = 0;
    while (std::getline(file, buffer))
    {
        int first_split = buffer.find(',');
        int second_split = buffer.find(',', first_split + 1);

        string name = buffer.substr(first_split + 1, second_split - first_split - 1);
        string age = buffer.substr(second_split + 1);
        record[i].columns["name"] = name;
        record[i].columns["age"] = age;
        dataBase.insertRecord(record[i]);
        ++i;
    }

    int option = 0;
    std::cin >> option;
    dataBase.commit();
    if (option == 1)
    {
        dataBase.crashRecovery();
        cout << "crash recovery finished" << endl;
    }

    for (auto &[id, record] : dataBase.write_set)
    {
        cout << "id " << id << " name " << record.columns["name"] << endl;
    }

    // cout << "commit result " << dataBase.commitTest() << endl;
    // dataBase.crashRecovery();

    if (option == 1)
    {
        return 0;
    }

    DataBase::Record new_record[50];
    for (int j = 0; j < 1000000; ++j)
    {
        cout << j << endl;
        for (int i = 0; i < 50; ++i)
        {
            new_record[i] = record[i];
            new_record[i].columns["age"] = j;
            dataBase.updateRecord(record[i], new_record[i]);
        }
    }

    return 0;

    cout << "start " << endl;
    for (auto &[name_value_pair, id_set] : dataBase.column_index)
    {
        for (auto &id : id_set)
        {
            cout << "name " << name_value_pair.first << " value " << name_value_pair.second << " id " << id << endl;
        }
    }
    for (auto itr = dataBase.primary_index.begin(); itr != dataBase.primary_index.end(); ++itr)
    {
        cout << "id " << itr->second.id << " " << itr->second.columns["name"] << " " << itr->second.columns["age"] << endl;
    }
    cout << "end" << endl;
    return 0;

    return 0;

    vector<DataBase::Record> vec;
    dataBase.readRecord(record.columns, vec);

    // dataBase.deleteRecord(vec[0]);

    dataBase.commit();

    // dataBase.readRecord(record2.columns, vec);

    DataBase::Record record3 = vec[0];
    record3.columns["name"] = "zyugemuzyugemu";
    dataBase.updateRecord(vec[0], record3);

    dataBase.commit();

    for (auto &[id, rec] : dataBase.primary_index)
    {
        cout << rec.columns["name"] << endl;
    }

    std::string message;
    // dataBase.redoLog->readRedoLog(message);

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
