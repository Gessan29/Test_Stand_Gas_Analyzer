#include "um_receiver_base.h"
#include <QDateTime>
#include <QDataStream>

um_receiver_base::um_receiver_base(QObject * parent)
    : QObject(parent)
{}

um_receiver_base::~um_receiver_base()
{}

void um_receiver_base::decode_packet(const QByteArray & dgData)
{
    auto hdr = um_header::from_raw((const uint8_t*)dgData.data());
    QDataStream s(dgData);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);
    s.skipRawData(4);

    if(hdr.cmdGroup == um_cmd_group::device)
        decode_as_cmd_from_device_group(hdr, s);
    if(hdr.cmdGroup == um_cmd_group::algorithm)
        decode_as_cmd_from_alg_group(hdr, s);
    if(hdr.cmdGroup == um_cmd_group::settings)
        decode_as_cmd_from_settings_group(hdr, s);
}

void um_receiver_base::decode_as_cmd_from_device_group(um_header hdr, QDataStream & s)
{
    if(hdr.cmdType == um_cmd_type::get_param)
    {
        if(hdr.devParam == um_dev_param::mcu_id)
            decode_mcu_id(s);
        if(hdr.devParam == um_dev_param::hw_version)
            decode_hw_version(s);
        if(hdr.devParam == um_dev_param::sw_version)
            decode_sw_version(s);
    }
    if(hdr.cmdType == um_cmd_type::notification)
    {
        if(hdr.devNotify == um_dev_notify::error)
        {
            um_error err;
            s >> err;
            emit add_error_record(
                 std::make_shared<data_logger_error_record>(
                            data_logger_error_record {
                                QDateTime::currentDateTime(), err } ) );
            emit error_occured(err);
        }
    }
}

void um_receiver_base::decode_mcu_id(QDataStream & s)
{
    QByteArray arr(12, 0);
    s.readRawData(arr.data(), 12);
    emit got_mcu_id(arr);
}

void um_receiver_base::decode_sw_version(QDataStream & s)
{
    quint8 major, minor;
    s >> minor;
    s >> major;
    emit got_sw_version(minor, major);
}

void um_receiver_base::decode_hw_version(QDataStream & s)
{
    quint8 major, minor;
    s >> minor;
    s >> major;
    emit got_hw_version(minor, major);
}

void um_receiver_base::decode_as_cmd_from_alg_group(um_header hdr, QDataStream & s)
{
    if(hdr.cmdType == um_cmd_type::control)
    {
        emit alg_cmd_executed(hdr.algCtrlCmd, hdr.status);
    }
    if(hdr.cmdType == um_cmd_type::set_param)
    {
        emit alg_param_set(hdr.algParam, hdr.status);
    }
    if(hdr.cmdType == um_cmd_type::get_param)
    {
        uint8_t value;
        s >> value;
        if(hdr.algParam == um_alg_param::state)
            emit got_alg_state(static_cast<um_alg_state>(value));
        if(hdr.algParam == um_alg_param::leak_level)
            emit got_leak_level(static_cast<um_leak_level>(value));
        if(hdr.algParam == um_alg_param::temp_mode)
            emit got_temp_mode(static_cast<um_temp_mode>(value));
        if(hdr.algParam == um_alg_param::calc_mode)
            emit got_calc_mode(static_cast<um_calc_mode>(value));
    }
    if(hdr.cmdType == um_cmd_type::notification)
    {
        if(hdr.algNotify == um_alg_notify::event)
            decode_alg_event(s);
        else if(hdr.algNotify == um_alg_notify::line_progress)
            decode_progress(um_progress::line, s);
        else if(hdr.algNotify == um_alg_notify::temp_progress)
            decode_progress(um_progress::temp, s);
        else if(hdr.algNotify == um_alg_notify::base_progress)
            decode_progress(um_progress::base, s);
        else if(hdr.algNotify == um_alg_notify::result)
            decode_alg_result(s);
        else
            decode_alg_vector(hdr.algNotify, s);
    }
}

void um_receiver_base::decode_alg_event(QDataStream & s)
{
    um_alg_event event;
    s >> event;
    if(event == um_alg_event::temp_stable)
        emit event_occured(um_event::temp_stable);
    if(event == um_alg_event::line_stable)
        emit event_occured(um_event::line_stable);
    if(event == um_alg_event::base_written)
        emit event_occured(um_event::base_written);
}

void um_receiver_base::decode_progress(um_progress pr, QDataStream & s)
{
    uint8_t value;
    s >> value;
    emit progress_updated(pr, value);
}

void um_receiver_base::decode_alg_result(QDataStream & s)
{
    auto algData = std::make_shared<um_data>();
    auto & ref = *algData;
    // Штамп времени
    ref.dateTime = QDateTime::currentDateTime();
    // Заголовок
    s >> ref.dataIndex;
    // Усреднение
    s >> ref.amplRef;
    s >> ref.ampl;
    s >> ref.zero;
    s >> ref.sun;
    // Температура
    s >> ref.temperature;
    s >> ref.peltierCurrent;
    s >> ref.peltierVoltage;
    s >> ref.powerVolateg;
    s >> ref.line;
    s >> ref.peltierUset;
    // Дистанция
    s >> ref.Range;
    s >> ref.diFast;
    s >> ref.diSlow;
    s >> ref.fixDist;
    // Корреляция
    s >> ref.peakRef;
    s >> ref.valleyRef;
    s >> ref.peakAnl;
    s >> ref.valleyAnl;
    s >> ref.corrPeak;
    s >> ref.corrValley;
    s >> ref.corrCross;
    s >> ref.corrLog;
    s >> ref.corrFlt;
    s >> ref.corrCoef;
    s >> ref.correlation;
    // Предконцентрация
    s >> ref.kapR;
    s >> ref.kappaR;
    s >> ref.crlr;
    s >> ref.slope;
    s >> ref.fastAtmCont;
    s >> ref.slowAtmCont;
    // Быстрая концентрация
    s >> ref.fullPrimeCont;
    s >> ref.fullNlkSN;
    s >> ref.fullSN;
    s >> ref.fullCont;
    s >> ref.fullNoise;
    s >> ref.fastPrimeCont;
    s >> ref.fastNlkSN;
    s >> ref.fastSN;
    s >> ref.fastCont;
    s >> ref.fastChkCont;
    // Медленная концентрация
    s >> ref.slowNlkSN;
    s >> ref.slowSN;
    s >> ref.slowCont;
    s >> ref.slowChkCont;
    s >> ref.leakCont;
    s >> ref.maxCorrelation;
    s >> ref.meanFullCont;
    s >> ref.meanFullSN;
    s >> ref.meanAmpl;
    s >> ref.meanZero;
    s >> ref.maxFastCont;
    // GPS
    s >> ref.gpsGeoData.msec;
    s >> ref.gpsGeoData.year;
    s >> ref.gpsGeoData.month;
    s >> ref.gpsGeoData.day;
    s >> ref.gpsGeoData.hour;
    s >> ref.gpsGeoData.min;
    s >> ref.gpsGeoData.sec;
    s >> ref.gpsGeoData.satCount;
    s >> ref.gpsGeoData.dataValid;
    s.skipRawData(1);
    s >> ref.gpsGeoData.latitude;
    s >> ref.gpsGeoData.longitude;
    s >> ref.gpsGeoData.altitude;
    s >> ref.gpsGeoData.speed;
    s >> ref.gpsGeoData.angle;
    s >> ref.gpsGeoData.error;
    s >> ref.gpsX;
    s >> ref.gpsY;
    s >> ref.gpsDistance;
    // Разные
//    s >> ref.maxIndex;
    s >> ref.sortPulses;
    s >> ref.sumSortPulses;
    s >> ref.dataFlags;
    // Leak line
    if(ref.dataFlags & (1 << int(um_data_flag::slow_updated))) {
        for(auto & it : algData->leakLine) {
            s >> it;
        }
        emit add_slow_record(make_record<data_logger_slow_record>(ref));
        if(ref.dataFlags & (1 << int(um_data_flag::wr_detr))) {
            emit add_detr_record(make_record<data_logger_detr_record>(ref));
        }
    } else {
        emit add_fast_record(make_record<data_logger_fast_record>(ref));
    }

    emit data_ready(algData);
}

void um_receiver_base::decode_alg_vector(um_alg_notify nt, QDataStream & s)
{
    int size = ( (nt == um_alg_notify::vector_nrm_anl) or
                 (nt == um_alg_notify::vector_nrm_ref) or
                 (nt == um_alg_notify::vector_test_adc ) ) ? 200 : 123;
    std::vector<float> pulse;
    pulse.resize(size);
    for(int i = 0; i < size; i++)
        s >> pulse[i];
    emit vector_received(static_cast<um_vector_id>(static_cast<int>(nt)-1), pulse);
}

void um_receiver_base::decode_as_cmd_from_settings_group(um_header hdr, QDataStream & s)
{
    if(hdr.cmdType == um_cmd_type::control)
    {
        um_settings_type type;
        s >> type;
        emit settings_cmd_executed(hdr.status, type, hdr.settingsCmd);
    }
    if(hdr.cmdType == um_cmd_type::set_param)
    {
        emit settings_set(hdr.status, hdr.settingsType);
    }
    if(hdr.cmdType == um_cmd_type::get_param)
    {
        if(hdr.settingsType == um_settings_type::algorithm_settings)
            decode_alg_settings(s);
        else if(hdr.settingsType == um_settings_type::work_mode_settings)
            decode_work_settings(s);
        else if(hdr.settingsType == um_settings_type::test_mode_settings)
            decode_test_settings(s);
        else if(hdr.settingsType == um_settings_type::link_settings)
            decode_link_settings(s);
        else if(hdr.settingsType == um_settings_type::laser)
            decode_laser_settings(s);
        else if(hdr.settingsType == um_settings_type::distance)
            decode_dist_settings(s);
        else if(hdr.settingsType == um_settings_type::gps)
            decode_gps_settings(s);
    }
}

void um_receiver_base::decode_alg_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_algorithm_settings>();
    s >> sets->pulsefilterCriteria;
    s >> sets->shift2;
    s >> sets->shiftCoefA;
    s >> sets->shiftCoefB;
    s >> sets->shiftCoefC;
    s >> sets->minPosCoef;
    s >> sets->minPosLowLimit;
    s >> sets->minPosHighLimit;
    s >> sets->maxPosCoef;
    s >> sets->maxPosLowLimit;
    s >> sets->maxPosHighLimit;
    s >> sets->delta;
    s >> sets->deltaLowLimit;
    s >> sets->deltaHighLimit;
    emit got_alg_settings(sets);
}

void um_receiver_base::decode_work_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_work_mode_settings>();
    for(size_t i = 0; i < um_leak_level_amount; i++)
    {
        s >> sets->leakLevelParams[i].corr;
        s >> sets->leakLevelParams[i].norm;
    }
    for(size_t i = 0; i < um_temp_mode_amount; i++)
    {
        s >> sets->tempModeParams[i].laserWfm.zeroLevel;
        s >> sets->tempModeParams[i].laserWfm.beginLevel;
        s >> sets->tempModeParams[i].laserWfm.endLevel;
        s >> sets->tempModeParams[i].laserWfm.beginTime;
        s >> sets->tempModeParams[i].laserWfm.endTime;
        s >> sets->tempModeParams[i].workTemp;
        s >> sets->tempModeParams[i].workLine;
        s >> sets->tempModeParams[i].factorWS;
    }
    s >> sets->regulatorParam.kp;
    s >> sets->regulatorParam.ki;
    s >> sets->regulatorParam.maxSetDiff;
    s >> sets->regulatorParam.lineToTempCoef;
    s >> sets->regulatorParam.switchToLineThr;
    s >> sets->regulatorParam.lineStableThr;
    s >> sets->tempTimeout;
    s >> sets->lineTimeout;
    s >> sets->autotestTime;
    s.skipRawData(2);
    s >> sets->refFactor;
    emit got_work_settings(sets);
}

void um_receiver_base::decode_test_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_test_mode_settings>();
    s >> sets->laserWfm.zeroLevel;
    s >> sets->laserWfm.beginLevel;
    s >> sets->laserWfm.endLevel;
    s >> sets->laserWfm.beginTime;
    s >> sets->laserWfm.endTime;
    s >> sets->workTemp;
    s >> sets->workLine;
    s >> sets->regParam.kp;
    s >> sets->regParam.ki;
    s >> sets->regParam.maxSetDiff;
    s >> sets->regParam.lineToTempCoef;
    s >> sets->regParam.switchToLineThr;
    s >> sets->regParam.lineStableThr;
    uint8_t tmp;
    s >> tmp;
    sets->control = static_cast<um_test_mode_control>(tmp);
    emit got_test_settings(sets);
}

void um_receiver_base::decode_link_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_link_settings>();
    quint32 rawAddr;
    s >> rawAddr; sets->manualIPAddres.setAddress(rawAddr);
    s >> rawAddr; sets->manualSubnetMask.setAddress(rawAddr);
    s >> rawAddr; sets->manualGateway.setAddress(rawAddr);
    s >> sets->listenPort;
    s >> sets->senderPort;
    s.readRawData((char*)sets->macAddr, 6);
    s.skipRawData(2);
    s >> sets->buadrate;
    s >> sets->ethSendFlags;
    s >> sets->rs232SendFlags;
    emit got_link_settings(sets);
}

void um_receiver_base::decode_laser_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_laser_settings>();
    s >> sets->maxLaserCurrent;
    s >> sets->maxPeltierCurrent;
    s >> sets->minTemp;
    s >> sets->maxTemp;
    s >> sets->thermA;
    s >> sets->thermB;
    s >> sets->thermC;
    emit got_laser_settings(sets);
}

void um_receiver_base::decode_dist_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_distance_settings>();
    for(auto & it : sets->r15)
        s >> it;
    for(auto & it : sets->r35)
        s >> it;
    for(auto & it : sets->coef)
        s >> it;
    s >> sets->rfactor;
    s >> sets->slopeShift;
    s >> sets->rangeLowLimit;
    s >> sets->rangeHighLimit;
    emit got_dist_settings(sets);
}

void um_receiver_base::decode_gps_settings(QDataStream & s)
{
    auto sets = std::make_shared<um_gps_settings>();
    s >> sets->baudrate;
    s >> sets->startupTimeout;
    s >> sets->configStringTimeout;
    for(auto & it : sets->configString)
    {
        s >> it.len;
        s.readRawData(it.buf, um_gps_settings::config_str_max_len);
    }
    emit got_gps_settings(sets);
}

data_logger_fast_record um_receiver_base::make_fast_record(const um_data & ref)
{
    return {
        ref.dateTime,
        ref.fastCont,
        ref.ampl,
        ref.fastSN,
        ref.zero,
        ref.sortPulses,
        ref.fastNlkSN,
        ref.diFast,
        ref.correlation
    };
}

data_logger_slow_record um_receiver_base::make_slow_record(const um_data & ref)
{
    return {
         {
            ref.dateTime,
            ref.fastCont,
            ref.ampl,
            ref.fastSN,
            ref.zero,
            ref.sortPulses,
            ref.fastNlkSN,    // fast itg
            ref.diFast,
            ref.correlation,
        },
        ref.temperature,
        ref.line,
        ref.peltierCurrent,
        ref.amplRef,
        ref.meanAmpl,
        ref.meanZero,
        ref.sumSortPulses,
        ref.diSlow,
        ref.maxCorrelation,
        ref.slowCont,
        ref.slowSN,
        ref.slowNlkSN,    // slow itg
        ref.leakCont,
        bool(ref.dataFlags & (1 << int(um_data_flag::leak_flag))),
        QDateTime(QDate(ref.gpsGeoData.year,
                        ref.gpsGeoData.month,
                        ref.gpsGeoData.day),
                  QTime(ref.gpsGeoData.hour,
                        ref.gpsGeoData.min,
                        ref.gpsGeoData.sec,
                        ref.gpsGeoData.msec)),
        ref.gpsGeoData.latitude,
        ref.gpsGeoData.longitude,
        ref.gpsGeoData.altitude,
        ref.gpsGeoData.speed,
        ref.gpsGeoData.angle,
        ref.gpsGeoData.error,
        ref.gpsX,
        ref.gpsY
    };
}

data_logger_detr_record um_receiver_base::make_detr_record(const um_data & ref)
{
    return { ref.dateTime, ref.leakLine };
}

template<class T>
std::shared_ptr<T> um_receiver_base::make_record(const um_data & ref)
{
    if constexpr (std::is_same_v<T, data_logger_fast_record>) {
        return std::make_shared<T>(make_fast_record(ref));
    }
    if constexpr (std::is_same_v<T, data_logger_slow_record>) {
        return std::make_shared<T>(make_slow_record(ref));
    }
    if constexpr (std::is_same_v<T, data_logger_detr_record>) {
        return std::make_shared<T>(make_detr_record(ref));
    }
}
