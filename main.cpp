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

    // Record�̒ǉ�(x 2)
    record1.columns["name"] = "�R�c";
    record1.columns["age"] = "12";
    dataBase.insertRecord(record1);

    record2.columns["name"] = "�c��";
    record2.columns["age"] = "24";
    dataBase.insertRecord(record2);

    record2.id = record1.id;
    dataBase.updateRecord(record1, record2);
    dataBase.deleteRecord(record1);

    // �ۑ�����Ă��邩�̊m�F
    // cout << dataBase.table[0].id << " " << dataBase.table[0].columns["name"] << dataBase.table[0].columns["age"] << endl;

    // vector<DataBase::Record> vec;   // �����ɍ���Record���i�[���邽�߂̕ϐ�
    // map<string, string> conditions; // ����
    // conditions["name"] = "�c��";    // ����1(name���c���ł���Ƃ�������)
    // dataBase.readRecord(conditions, vec);

    // for (auto &value : vec)
    // {
    //     // �����ɍ���Record��age���m�F
    //     cout << value.columns["age"] << endl;
    // }
}
