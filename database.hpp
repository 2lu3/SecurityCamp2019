#ifndef SECURITYCAMP2019_DATABASE_HPP_
#define SECURITYCAMP2019_DATABASE_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>

class RedoLog;

class DataBase
{
public:
    /*  ����
        1: id�̓��j�[�N�ł���
        2: id��0�ȊO�ł���
        3: columns��key�͂��ׂĂ�Record�ŋ��ʂłȂ���΂Ȃ�Ȃ�
     */
    struct Record
    {
        const static std::uint64_t kIdNull = 0;
        const static std::string kValueNull;
        std::uint64_t id;
        std::map<std::string, std::string> columns;

        Record();
    };
    const static int table_max_num = 100; // �ێ��ł���Record�̍ő�l(�f�o�b�O�p��public�ɂ����Ă��邪�Aprivate�Ɉڂ��\��)
    Record table[table_max_num];          // �f�[�^�x�[�X�̃f�[�^���̂���(�f�o�b�O�p��public�ɂ����Ă��邪�Aprivate�Ɉڂ��\��)
    const static int kSuccess = 1;        // �֐��̖߂�l�ŁA������\��
    const static int kFailure = 0;        // �֐��̖߂�l�ŁA���s��\��
    DataBase();
    int crashRecovery();                                                                  // ������
    int begin();                                                                          // ������
    int updateRecord(std::uint64_t id, const Record &update_record_condition);            // id�Ŏw�肵��Record��update_record_condition�ɒu��������
    int updateRecord(const Record &target_record, const Record &update_record_condition); // target_record�Ŏw�肵��Record��update_record_condition�ɒu��������
    int deleteRecord(const Record &target_record);                                        // target_record�Ŏw�肵��Record����������
    int deleteRecord(std::uint64_t id);                                                   // id�Ŏw�肵��Record����������
    int commit();                                                                         // ������
    int abort();                                                                          // ������
    int createKey(std::string columns[]);                                                 // ������

    int readRecord(const std::map<std::string, std::string> &target_columns, std::vector<Record> &return_records); // target_columns�Ŏw�肵�������ɍ���Record��Ԃ�
    int readRecord(std::uint64_t id, Record &return_record);                                                       // id�Ŏw�肵��Record��Ԃ�
    int insertRecord(Record &new_record);                                                                          // new_record�̃R�s�[��table�ɒǉ�����
    int setId2Record(Record &target_record);                                                                       // target_record�ɂ܂��o�^����Ă��Ȃ�ID��ݒ肷��(���̊֐������Ȃ��ƁA���̊֐��̈�����const�����邱�Ƃ��ł��Ȃ�)

private:
    std::unique_ptr<RedoLog> redoLog;

    // ���݂́Atable�Ɋi�[����Ă���Record�̐�
    int table_num = 0;

    std::random_device rnd;

    const int CHECK_RECORD_OPTION_INSERT = 0;
    const int CHECK_RECORD_OPTION_UPDATE = 1;
    int checkRecord(const Record &check_record, int option); // Record������Ɏ��܂��Ă��邩�`�F�b�N����

    std::set<std::string> column_names{"name", "age"};
    std::map<std::uint64_t, std::uint32_t> primary_index; // id-table�̓Y�����i�[����
};

class UndoLog
{
public:
private:
};
class RedoLog
{
public:
    // Insert���L�^����
    int addInsertLog(const DataBase::Record &record, int table_index);
    // Update�̑O��̍������L�^���� (before_record:�ύX�O, updated_record:�ύX��)
    int addUpdateLog(const DataBase::Record &before_record, const DataBase::Record &updated_record);
    // Delete���L�^����
    int addDeleteLog(int id);

    const int kSuccess = DataBase::kSuccess; // �����������֐��̖߂�l
    const int kFaliure = DataBase::kFailure; // ���s�������֐��̖߂�l

private:
    const char log_file_name[10] = "redo.log";
};

#endif //SECURITYCAMP2019_DATABASE_HPP_
