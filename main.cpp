#include <iostream>
#include "database.hpp"

using std::cout;
using std::endl;

int main()
{
    system("chcp 932");
    DataBase dataBase;
    DataBase::Record record;
    dataBase.readRecord("“¡“c", &record);
    cout << record.id << " " << record.name << " " << record.money << endl;
}
