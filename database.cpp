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
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::vector;

#define FOR for
#define rep(i, n) FOR(int64_t i = 0; i < n; ++i)
// ���݂̊֐������擾
#define FUNCNAME __FUNCTION__

DataBase::DataBase()
{
    // Record������

    for (int i = 0; i < table_num; ++i)
    {
        table[i].id = 0;
    }

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
    vector<int> selected_table_index;
    // �ŏ��́A0~���R�[�h�̐��܂ł��ׂ�
    rep(i, table_num)
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

// id : �ύX����O��Record��id
// update_record_condition : �ύX���Record
// target_record���g�p���āAtable�ł̓Y�����擾���Aupdate_record_condition��������
int DataBase::updateRecord(uint64_t id, const Record &update_record_condition)
{
    // id���ύX����Ă��Ȃ����`�F�b�N
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
    uint32_t target_table_index = primary_index[id];
    // �ύX����Ώۂ̓Y���ɁAupdate_record_condition����
    table[target_table_index] = update_record_condition;
    return kSuccess;
}
// target_record : �ύX����O��Record
// update_record_condition : �ύX���Record
// target_record���g�p���āAtable�ł̓Y�����擾���Aupdate_record_condition��������
int DataBase::updateRecord(const Record &target_record, const Record &update_record_condition)
{
    if (target_record.id == 0)
    {
        return kFailure;
    }
    return updateRecord(target_record.id, update_record_condition);
}

// check_record�ŗ^����ꂽRecord������Ɏ��܂��Ă��邩�`�F�b�N����
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
    // insert�̂Ƃ��́A������id������U��
    if (option == CHECK_RECORD_OPTION_INSERT)
    {
        int random_id = rnd();
        while (primary_index.count(random_id) == 1)
        {
            random_id = rnd();
        }
    }
    else // option == CHECK_RECORD_OPTION_UPDATE
    {
        if (check_record.id == 0)
        {
            return kFailure;
        }
    }

    // ����3: column�ɓo�^����Ă���(map�^�ł�)key�̏W���́Acolumn_names�Ɋi�[����Ă��镶����̏W���Ɠ����łȂ���΂Ȃ�Ȃ�
    // insert, update ����

    // check_record�ɂ��邪�Acolumn_names�ɂ͂Ȃ����̂�T��
    for (const auto &[key, value] : check_record.columns)
    {
        // column_names�ɓo�^����Ă��邩�`�F�b�N
        if (column_names.count(key) != 1)
        {
            cerr << "key '" << key << "' is not registered in column_names" << endl;
            return kFailure;
        }
    }

    // column_names�ɂ��邪�Acheck_record�ɂ͂Ȃ����̂�T��
    for (const auto &key : column_names)
    {
        // check_record�ɓo�^����Ă��邩�`�F�b�N
        if (check_record.columns.count(key) != 1)
        {
            cerr << "key '" << key << "' is not registered in record" << endl;
            return kFailure;
        }
    }

    return kSuccess;
}

// target_record�Ŏw�肵��Record���폜����
int DataBase::deleteRecord(const Record &target_record)
{
    return deleteRecord(target_record.id);
}
// target_record�Ŏw�肵��Record���폜����
int DataBase::deleteRecord(uint64_t id)
{
    // target_record��id����
    uint32_t target_table_index = primary_index[id];
    for (int i = target_table_index; i < table_num - 1; ++i)
    {
        table[i] = table[i + 1];
    }
    --table_num;
    primary_index.erase(target_table_index);
    return kSuccess;
}

// table[]�̖����ɐV����Record��ǉ�
int DataBase::insertRecord(const Record &new_record)
{
    // Record�̐���`�F�b�N
    if (checkRecord(new_record, CHECK_RECORD_OPTION_INSERT) == kFailure)
    {
        cerr << FUNCNAME << "(): Failed to insert Record" << endl;
        return kFailure;
    }
    table[table_num] = new_record;
    primary_index[new_record.id] = table_num;
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
