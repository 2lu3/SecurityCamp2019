#include <iostream>
#include <vector>
#include <map>
#include <string>
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
    DataBase::Record record;
    record.columns["name"] = "�R�c";
    record.columns["age"] = "12";
    dataBase.insertRecord(record);
    cout << dataBase.table[0].columns["name"] << dataBase.table[0].columns["age"] << endl;

    vector<DataBase::Record> vec;
    map<string, string> conditions;
    conditions["name"] = "�R�c";
    dataBase.readRecord(conditions, &vec);

    for (auto &value : vec)
    {
        cout << value.columns["name"] << endl;
    }
}
