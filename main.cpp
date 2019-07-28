#include <iostream>
#include "database.hpp"

using std::cout;
using std::endl;

int main()
{
    system("chcp 932");
    DataBase dataBase;
    DataBase::Record record;
    record.columns["name"] = "ŽR“c";
    record.columns["age"] = "12";
    dataBase.insertRecord(record);
    cout << dataBase.table[0].columns["name"] << dataBase.table[0].columns["age"] << endl;
}
