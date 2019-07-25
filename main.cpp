#include <iostream>
#include "database.hpp"

using std::cout;
using std::endl;

int main()
{
    DataBase dataBase;
    DataBase::Record record;
    dataBase.readRecord("藤田", &record);
    // cout << endl
    //      << endl
    //      << endl;
    // cout << record.id << " " << record.name << " " << record.money << endl;
}
