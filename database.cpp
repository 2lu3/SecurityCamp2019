#include "database.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::extent;
using std::int64_t;
using std::map;
using std::ofstream;
using std::string;
using std::to_string;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::vector;

// ���݂̊֐������擾
#define FUNCNAME __FUNCTION__

// Insert��RedoLog��redo.log�ɋL�^����
int RedoLog::addInsertLog(const DataBase::Record &record, int table_index)
{
    // �����} : INSERT(\x1f)table�ł̓Y��(\x1f)id(\x1f)key(\x1f)value(\x1f)...(\x1f)key(\x1f)value(\x1e)
    // RedoLog�̓��e
    string log_message = "INSERT" + '\x1f' + to_string(table_index) + '\x1f' + to_string(record.id);
    for (const auto &[key, value] : record.columns)
    {
        log_message += '\x1f' + key + '\x1f' + value;
    }
    log_message += '\x1e';

    // �t�@�C���ւ̏�������
    ofstream file(log_file_name, std::ios::app);
    if (file)
    {
        file << log_message << endl;
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }

    return kSuccess;
}

// Update��RedoLog��redo.log�ɋL�^����
int RedoLog::addUpdateLog(const DataBase::Record &before_record, const DataBase::Record &updated_record)
{
    // �����} : UPDATE(\x1f)id(\x1f)�ύX����key(\x1f)�ύX����value(\x1f)...(\x1f)�ύX����key(\x1f)�ύX����value(\x1e)
    // RedoLog�̓��e
    string log_message = "UPDATE " + '\x1f' + to_string(updated_record.id);
    auto before_record_iterator = before_record.columns.begin();
    auto updated_record_iterator = updated_record.columns.begin();
    for (;
         before_record_iterator != before_record.columns.end();
         ++before_record_iterator, ++updated_record_iterator)
    {
        if (before_record_iterator->second != updated_record_iterator->second)
        {
            log_message += '\x1f' + updated_record_iterator->first + '\x1f' + updated_record_iterator->second;
        }
    }
    log_message += '\x1e'; // �t�@�C���ւ̏�������
    ofstream file(log_file_name, std::ios::app);
    if (file)
    {
        file << log_message << endl;
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }

    return kSuccess;
}

int RedoLog::addDeleteLog(int id)
{
    // �����} : DELETE(\x1f)id(\x1e)
    string log_message = "DELETE" + '\x1f' + to_string(id) + '\x1e';

    ofstream file(log_file_name, std::ios::app);
    if (file)
    {
        file << log_message << endl;
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFaliure;
    }
    return kSuccess;
}

DataBase::Record::Record()
{
    // id��null�l�ŏ�����
    id = kIdNull;
}

DataBase::DataBase() : redoLog(new RedoLog)
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
    cout << columns[0] << endl;
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
    vector<int> selected_table_index;
    // �ŏ��́A0~���R�[�h�̐��܂ł��ׂ�
    for (int i = 0; i < table_num; ++i)
    {
        selected_table_index.emplace_back(i);
    }

    // �����̏������A���ԂɊm�F���Ă���
    for (const auto &[column_name, column_value] : target_columns)
    {
        // column_name�����݂��邩�`�F�b�N
        // column_names�ɂ́A���ݎg���Ă���column�̖��O�����ׂĊi�[����Ă���
        if (column_names.count(column_name) == 1)
        {
            // �����ɂ���Ȃ�Record�̓Y����selected_table_index����������Ă���
            for (auto it = selected_table_index.begin(); it != selected_table_index.end(); ++it)
            {
                // �����A�l����v���Ȃ��ꍇ�́A�����ɍ����Ă��Ȃ��Ƃ�������
                if (table[*it].columns[column_name] != column_value)
                {
                    // selected_table_index����폜
                    selected_table_index.erase(it);
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
    for (auto it = selected_table_index.begin(); it != selected_table_index.end(); ++it)
    {
        return_records.emplace_back(table[*it]);
    }
    return kSuccess;
}

// Id�Ŏw�肵��Record��return_record�ɑ������
int DataBase::readRecord(uint64_t id, Record &return_record)
{
    if (id == Record::kIdNull)
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }

    if (auto iterator = primary_index.find(id); iterator != primary_index.end())
    {
        return_record = table[iterator->second];
        return kSuccess;
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }
}

// id : �ύX����O��Record��id
// update_record_condition : �ύX���Record
// target_record���g�p���āAtable�ł̓Y�����擾���Aupdate_record_condition��������
int DataBase::updateRecord(uint64_t id, const Record &update_record_condition)
{
    // id���ύX�O�ƕύX��ň�v���邩�`�F�b�N
    if (id != update_record_condition.id)
    {
        cerr << FUNCNAME << "(): id of target_record and update_record_condition is not same" << endl;
        return kFailure;
    }

    // update_record_condition������ɔ����Ă��Ȃ����`�F�b�N����
    if (checkRecord(update_record_condition, CHECK_RECORD_OPTION_UPDATE) == kFailure)
    {
        return kFailure;
    }

    // id����Aprimary_index���g���āAtable[]�̓Y�����擾
    if (auto target_table_index_iterator = primary_index.find(id); target_table_index_iterator != primary_index.end())
    {
        // �ύX����Ώۂ̓Y���ɁAupdate_record_condition����
        table[target_table_index_iterator->second];
        return kSuccess;
    }
    else
    {
        cerr << FUNCNAME << "(): Error" << endl;
        return kFailure;
    }
    return kSuccess;
}

// updateRecord(uint64_t id, const Record &update_record_condition)�̃I�[�o�[���[�h
int DataBase::updateRecord(const Record &target_record, const Record &update_record_condition)
{
    return updateRecord(target_record.id, update_record_condition);
}

// target_record.id�ɂ܂��o�^����Ă��Ȃ�ID��������
int DataBase::setId2Record(Record &target_record)
{
    // id��null���A���ł�id���o�^����Ă���ꍇ�́Aid��ύX����K�v������
    if (target_record.id == Record::kIdNull || primary_index.count(target_record.id) == 1)
    {
        uint64_t random_id = rnd();
        // while(id���o�^����Ă��� || id==null(=0))
        while (primary_index.count(random_id) == 1 || random_id == Record::kIdNull)
        {
            // random_id���X�V
            random_id = rnd();
        }
        // �܂��o�^����Ă��Ȃ�id����
        target_record.id = random_id;
    }
    return kSuccess;
}

// check_record�ŗ^����ꂽRecord������(database.hpp�ɋL��)�Ɏ��܂��Ă��邩�`�F�b�N����
// option: �`�F�b�N������e��Record��insert��update�Ŕ����ɈႤ 0:insert 1:update
int DataBase::checkRecord(const Record &check_record, int option)
{
    // ����1: id�̓��j�[�N�ł���
    // insert�̂Ƃ��̂�
    if (option == CHECK_RECORD_OPTION_INSERT)
    {
        // id�����łɓo�^����Ă���ꍇ
        if (primary_index.count(check_record.id) != 0)
        {
            return kFailure;
        }
    }

    // ����2: id��0�ȊO�ł���
    if (check_record.id == Record::kIdNull)
    {
        return kFailure;
    }

    // ����3: column�ɓo�^����Ă���(map�^�ł�)key�̏W���́Acolumn_names�Ɋi�[����Ă��镶����̏W���Ɠ����łȂ���΂Ȃ�Ȃ�
    // insert, update ����

    // �ŏ��ɗv�f�����ׂ�
    if (column_names.size() != check_record.columns.size())
    {
        // �v�f�����Ⴄ�ꍇ�A�����łȂ�
        return kFailure;
    }

    // ���Ԃ�key����v���邩�𒲂ׂ�
    // ���������̒��ŁAfor(auto column_names_iterator = column_names.begin(), auto column_iterator = check_record.columns.begin())�Ƃ���ƃG���[���o��
    auto column_names_iterator = column_names.begin();
    auto column_iterator = check_record.columns.begin();
    for (;
         column_names_iterator != column_names.end() && column_iterator != check_record.columns.end();
         ++column_iterator, ++column_names_iterator)
    {
        // column_names��key �� column��key����v���Ă��Ȃ��ꍇ
        if (*column_names_iterator != column_iterator->first)
        {
            return kFailure;
        }
    }

    return kSuccess;
}

// table[]�̖����ɐV����Record��ǉ�
// id������U�邽�߁Aconst Record &new_record�ɂ��Ă��Ȃ�
int DataBase::insertRecord(Record &new_record)
{
    // ID���Z�b�g
    setId2Record(new_record);
    // Record�̐���`�F�b�N
    if (checkRecord(new_record, CHECK_RECORD_OPTION_INSERT) == kFailure)
    {
        cerr << FUNCNAME << "(): Failed to insert Record" << endl;
        return kFailure;
    }
    redoLog->addInsertLog(new_record, table_num);
    // table[table_num] = new_record;
    // primary_index[new_record.id] = table_num;
    table_num++;
    return kSuccess;
}

// deleteRecord(uint64_t id)�̃I�[�o�[���[�h
int DataBase::deleteRecord(const Record &target_record)
{
    return deleteRecord(target_record.id);
}

// target_record�Ŏw�肵��Record���폜����
int DataBase::deleteRecord(uint64_t id)
{
    if (id == Record::kIdNull)
    {
        return kFailure;
    }
    // target_record��id����
    uint32_t target_table_index;
    if (auto iterator = primary_index.find(id); iterator != end(primary_index))
    {
        target_table_index = iterator->second;
    }
    else
    {
        return kFailure;
    }
    for (int i = target_table_index; i < table_num - 1; ++i)
    {
        table[i] = table[i + 1];
    }
    --table_num;
    primary_index.erase(target_table_index);
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
