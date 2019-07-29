#include "database.hpp"

#include <iostream>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::extent;
using std::fscanf;
using std::int64_t;
using std::map;
using std::string;
using std::to_string;
using std::uint64_t;
using std::uint8_t;
using std::vector;

#define FOR for
#define rep(i, n) FOR(int64_t i = 0; i < n; ++i)
// ���݂̊֐������擾
#define FUNCNAME getFuncName(__FUNCTION__)

// const char*�^����string�^�ւ�cast
inline string getFuncName(const char *name)
{
    return name;
}

DataBase::DataBase()
{
    FILE *fp = fopen("data.csv", "r");
    if (fp == NULL)
    {
        cout << "�t�@�C�����J���܂���" << endl;
    }
    fclose(fp);
}

int DataBase::begin()
{
    return kSuccess;
}

int DataBase::createKey(std::string columns[])
{
    return kSuccess;
}

/*
    target_columns�Ŏw�肵�������ɂ���Record(������)��Ԃ��֐�
    �p��:
    map<string, string> mp;
    mp["name"] = "�R�c";
    mp["gender"] = "male"; // �������w��
    vector<Record> record_receiver; // Record���󂯎�邽�߂�vector
    dataBase.readRecord(mp, record_receiver);
    for(auto &value : vec) {
        // value�́Amp�Ŏw�肵�������ɍ���Record
    }
*/
int DataBase::readRecord(const map<string, string> &target_columns, vector<Record> &return_records)
{
    // �i�荞��ł���Œ�/�i�荞��table�̓Y���̏W��
    vector<int> narrowed_table_index;
    // �ŏ��́A0~���R�[�h�̐��܂ł��ׂ�
    rep(i, table_num)
    {
        narrowed_table_index.emplace_back(i);
    }

    // �����̏������A���ԂɊm�F���Ă���
    for (const auto &[column_name, column_value] : target_columns)
    {
        // column_name�����݂��邩�`�F�b�N
        // column_names�ɂ́A���ݎg���Ă���column�̖��O�����ׂĊi�[����Ă���
        if (column_names.count(column_name) == 1)
        {
            // �����ɂ���Ȃ�Record�̓Y����narrowed_table_index����������Ă���
            for (auto it = narrowed_table_index.begin(); it != narrowed_table_index.end(); ++it)
            {
                // �����A�l����v���Ȃ��ꍇ�́A�����ɍ����Ă��Ȃ��Ƃ�������
                if (table[*it].columns[column_name] != column_value)
                {
                    // narrowed_table_index����폜
                    narrowed_table_index.erase(it);
                }
            }
        }
        else
        {
            // ���݂��Ȃ�column�̖��O���w�肳�ꂽ�ꍇ
            cerr << "Error: " << FUNCNAME << "(): there is no column_name '" << column_name << "' in column_names" << endl;
            return kFailure;
        }
    }

    // return_records�ɁA�i�荞��Record�������Ă���
    for (auto it = narrowed_table_index.begin(); it != narrowed_table_index.end(); ++it)
    {
        return_records.emplace_back(table[*it]);
    }
    return kSuccess;
}

int DataBase::updateRecord()
{
    return kSuccess;
}

int DataBase::deleteRecord()
{

    return kSuccess;
}

int DataBase::insertRecord(Record new_record)
{
    for (const auto &[key, value] : new_record.columns)
    {
        // column_names�ɓo�^����Ă��邩�`�F�b�N
        if (column_names.count(key) != 1)
        {
            new_record.columns.erase(key);
        }
    }
    table[table_num] = new_record;
    table_num++;
    return kSuccess;
}

int DataBase::commit()
{
    return kSuccess;
}

int DataBase::abort()
{
    return kSuccess;
}

int DataBase::crashRecovery()
{
    return kSuccess;
}
