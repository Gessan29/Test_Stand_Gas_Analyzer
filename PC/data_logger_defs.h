#ifndef DATA_LOGGER_DEFS_H
#define DATA_LOGGER_DEFS_H

#include <QDir>
#include <QString>
#include <QDateTime>
#include "um_defs.h"

struct data_logger_session
{
    QDir    dir;
    QString system;
    QString inspArea;
    QString operators;
    QString weather;
    QString customer;
    int     leakPauseCntMax;
    std::array<float,123> logRefSample;
};

struct data_logger_session_stop_data {
    um_leak_level_param leakLevelParam;
    um_calc_mode calcMode;
    float currDistance;
};

/**
 * @brief Структура данных для записи в файлы "fast"
 */
struct data_logger_fast_record
{
    QDateTime timestamp;
    float fastCont;
    float ampl;
    float fastSN;
    float zero;
    int   sortPulses;
    float fastNlkSN;    // fast itg
//  int   availSamplePerChannel;
    float diFast;
    float correlation;
};

/**
 * @brief Структура данных для записи в файлы "fast", "slow", "track и "leak"
 */
struct data_logger_slow_record
{
    data_logger_fast_record fast;
    float temperature;
    float linePos;
    float peltierCurrent;
    float refAmpl;
    float meanAmpl;
    float meanZero;
    int   sumSortPulses;
    float diSlow;
    float maxCorrelation;
    float slowCont;
    float slowSN;
    float slowNlkSN;    // slow itg
    float leakCont;
    bool  leakFlag;
    QDateTime gpsTime;
    float latitude;
    float longitude;
    float altitude;
    float speed;
    float angle;
    float gpsError;
    float x;
    float y;
};

/**
 * @brief Структура данных для записи в файл "detr"
 */
struct data_logger_detr_record
{
    QDateTime timestamp;
    std::array<float, 123> leakLine;
};

struct data_logger_note_record
{
    QDateTime timestamp;
    float     latitude;
    float     longitude;
    QString   comment;
};

struct data_logger_error_record
{
    QDateTime timestamp;
    um_error  error;
};

enum class data_logger_error
{
    dir_not_extists,
    cannot_create_or_open_session_file,
    cannot_create_fast_file,
    cannot_create_slow_file,
    cannot_create_track_file,
    cannot_create_detr_file,
    cannot_create_leak_file,
    cannot_create_note_file,
    cannot_create_error_file,
};

#endif // DATA_LOGGER_DEFS_H
