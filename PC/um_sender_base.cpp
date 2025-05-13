#include "um_sender_base.h"
#include <QDataStream>

um_sender_base::um_sender_base(QObject * parent)
    : QObject(parent)
    , dataToSend()
{
    dataToSend.reserve(1500);
}

um_sender_base::~um_sender_base() {}

void um_sender_base::get_device_param(um_dev_param par)
{
    make_dev_get_param_packet(par);
    send_packet(dataToSend);
}

void um_sender_base::exec_cmd(um_alg_cmd cmd)
{
    make_alg_cmd_packet(cmd);
    send_packet(dataToSend);
}

void um_sender_base::exec_cmd_adc_test( um_alg_cmd_data cmd_data )
{
    make_alg_cmd_packet( um_alg_cmd::test_adc, cmd_data );
    send_packet(dataToSend);
}

void um_sender_base::get_alg_param(um_alg_param par)
{
    make_alg_get_param_packet(par);
    send_packet(dataToSend);
}

void um_sender_base::set_leak_level(um_leak_level leakLevel)
{
    make_alg_set_leak_level_packet(leakLevel);
    send_packet(dataToSend);
}

void um_sender_base::set_temp_mode(um_temp_mode tempMode)
{
    make_alg_set_temp_mode_packet(tempMode);
    send_packet(dataToSend);
}

void um_sender_base::set_calc_mode(um_calc_mode calcMode)
{
    make_alg_set_calc_mode_packet(calcMode);
    send_packet(dataToSend);
}

void um_sender_base::set_alg_settings(std::shared_ptr<um_algorithm_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::set_work_settings(std::shared_ptr<um_work_mode_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::set_test_settings(std::shared_ptr<um_test_mode_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::set_link_settings(std::shared_ptr<um_link_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::set_laser_settings(std::shared_ptr<um_laser_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::set_dist_settings(std::shared_ptr<um_distance_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::set_gps_settings(std::shared_ptr<um_gps_settings> settings)
{
    make_set_settings_packet(*settings);
    send_packet(dataToSend);
}

void um_sender_base::get_settings(um_settings_type type)
{
    make_get_settings_packet(type);
    send_packet(dataToSend);
}

void um_sender_base::exec_settings_cmd(um_settings_cmd cmd, um_settings_type type)
{
    make_settings_cmd_packet(cmd, type);
    send_packet(dataToSend);
}


void um_sender_base::make_dev_get_param_packet(um_dev_param par)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::device   ));
    dataToSend.append( static_cast<char>( um_cmd_type::get_param ));
    dataToSend.append( static_cast<char>( par                    ));
    dataToSend.append( static_cast<char>( um_status::success     ));
}

void um_sender_base::make_alg_cmd_packet(um_alg_cmd cmd)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::algorithm ));
    dataToSend.append( static_cast<char>( um_cmd_type::control    ));
    dataToSend.append( static_cast<char>( cmd                     ));
    dataToSend.append( static_cast<char>( um_status::success      ));
}

void um_sender_base::make_alg_cmd_packet(um_alg_cmd cmd, um_alg_cmd_data cmd_data )
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::algorithm ));
    dataToSend.append( static_cast<char>( um_cmd_type::control    ));
    dataToSend.append( static_cast<char>( cmd                     ));
    dataToSend.append( static_cast<char>( um_status::success      ));
    dataToSend.append( static_cast<char>( cmd_data                ));
}

void um_sender_base::make_alg_set_leak_level_packet(um_leak_level leakLevel)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::algorithm  ));
    dataToSend.append( static_cast<char>( um_cmd_type::set_param   ));
    dataToSend.append( static_cast<char>( um_alg_param::leak_level ));
    dataToSend.append( static_cast<char>( um_status::success       ));
    dataToSend.append( static_cast<char>( leakLevel                ));
}

void um_sender_base::make_alg_set_temp_mode_packet(um_temp_mode tempMode)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::algorithm ));
    dataToSend.append( static_cast<char>( um_cmd_type::set_param  ));
    dataToSend.append( static_cast<char>( um_alg_param::temp_mode ));
    dataToSend.append( static_cast<char>( um_status::success      ));
    dataToSend.append( static_cast<char>( tempMode                ));
}

void um_sender_base::make_alg_set_calc_mode_packet(um_calc_mode calcMode)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::algorithm ));
    dataToSend.append( static_cast<char>( um_cmd_type::set_param  ));
    dataToSend.append( static_cast<char>( um_alg_param::calc_mode ));
    dataToSend.append( static_cast<char>( um_status::success      ));
    dataToSend.append( static_cast<char>( calcMode                ));
}

void um_sender_base::make_alg_get_param_packet(um_alg_param par)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::algorithm ));
    dataToSend.append( static_cast<char>( um_cmd_type::get_param  ));
    dataToSend.append( static_cast<char>( par                     ));
    dataToSend.append( static_cast<char>( um_status::success      ));
}

void um_sender_base::make_set_settings_packet(const um_algorithm_settings & sets)
{
    dataToSend.resize(sizeof(um_header) + sizeof (um_algorithm_settings));
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::algorithm_settings;
    s << um_status::success;
    s << sets.pulsefilterCriteria;
    s << sets.shift2;
    s << sets.shiftCoefA;
    s << sets.shiftCoefB;
    s << sets.shiftCoefC;
    s << sets.minPosCoef;
    s << sets.minPosLowLimit;
    s << sets.minPosHighLimit;
    s << sets.maxPosCoef;
    s << sets.maxPosLowLimit;
    s << sets.maxPosHighLimit;
    s << sets.delta;
    s << sets.deltaLowLimit;
    s << sets.deltaHighLimit;
}

void um_sender_base::make_set_settings_packet(const um_work_mode_settings & sets)
{
    dataToSend.resize(sizeof(um_header) + sizeof (um_work_mode_settings));
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::work_mode_settings;
    s << um_status::success;
    for(size_t i = 0; i < um_leak_level_amount; i++)
    {
        s << sets.leakLevelParams[i].corr;
        s << sets.leakLevelParams[i].norm;
    }
    for(size_t i = 0; i < um_temp_mode_amount; i++)
    {
        s << sets.tempModeParams[i].laserWfm.zeroLevel;
        s << sets.tempModeParams[i].laserWfm.beginLevel;
        s << sets.tempModeParams[i].laserWfm.endLevel;
        s << sets.tempModeParams[i].laserWfm.beginTime;
        s << sets.tempModeParams[i].laserWfm.endTime;
        s << sets.tempModeParams[i].workTemp;
        s << sets.tempModeParams[i].workLine;
        s << sets.tempModeParams[i].factorWS;
    }
    s << sets.regulatorParam.kp;
    s << sets.regulatorParam.ki;
    s << sets.regulatorParam.maxSetDiff;
    s << sets.regulatorParam.lineToTempCoef;
    s << sets.regulatorParam.switchToLineThr;
    s << sets.regulatorParam.lineStableThr;
    s << sets.tempTimeout;
    s << sets.lineTimeout;
    s << sets.autotestTime;
    s.skipRawData(2);
    s << sets.refFactor;
}

void um_sender_base::make_set_settings_packet(const um_test_mode_settings & sets)
{
    dataToSend.resize(sizeof(um_header) + sizeof (um_test_mode_settings));
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);
    quint32 tmp = static_cast<quint8>(sets.control);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::test_mode_settings;
    s << um_status::success;
    s << sets.laserWfm.zeroLevel;
    s << sets.laserWfm.beginLevel;
    s << sets.laserWfm.endLevel;
    s << sets.laserWfm.beginTime;
    s << sets.laserWfm.endTime;
    s << sets.workTemp;
    s << sets.workLine;
    s << sets.regParam.kp;
    s << sets.regParam.ki;
    s << sets.regParam.maxSetDiff;
    s << sets.regParam.lineToTempCoef;
    s << sets.regParam.switchToLineThr;
    s << sets.regParam.lineStableThr;
    s << tmp;
}

void um_sender_base::make_set_settings_packet(const um_link_settings & sets)
{
    auto ip_to_u32 = [](const QHostAddress & a)
    {
        auto str = a.toString();
        auto a1 = str.section(QChar('.'), 0, 0).toUInt();
        auto a2 = str.section(QChar('.'), 1, 1).toUInt();
        auto a3 = str.section(QChar('.'), 2, 2).toUInt();
        auto a4 = str.section(QChar('.'), 3, 3).toUInt();
        return (a1 << 24) | (a2 << 16) | (a3 << 8) | a4;
    };

    static constexpr auto size = sizeof(quint32)*3 + sizeof(quint16)*2 + 8 +
            sizeof(quint32) + sizeof(quint32)*2;

    dataToSend.resize(sizeof(um_header) + size);
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::link_settings;
    s << um_status::success;
    s << ip_to_u32(sets.manualIPAddres);
    s << ip_to_u32(sets.manualSubnetMask);
    s << ip_to_u32(sets.manualGateway);
    s << sets.listenPort;
    s << sets.senderPort;
    s.writeRawData((const char*)sets.macAddr, 6);
    s.skipRawData(2);
    s << sets.buadrate;
    s << sets.ethSendFlags;
    s << sets.rs232SendFlags;
}

void um_sender_base::make_set_settings_packet(const um_laser_settings & sets)
{
    dataToSend.resize(sizeof(um_header) + sizeof (um_laser_settings));
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::laser;
    s << um_status::success;

    s << sets.maxLaserCurrent;
    s << sets.maxPeltierCurrent;
    s << sets.minTemp;
    s << sets.maxTemp;
    s << sets.thermA;
    s << sets.thermB;
    s << sets.thermC;
}

void um_sender_base::make_set_settings_packet(const um_distance_settings & sets)
{
    dataToSend.resize(sizeof(um_header) + sizeof(um_distance_settings));
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::distance;
    s << um_status::success;
    for(const auto & it : sets.r15)
        s << it;
    for(const auto & it : sets.r35)
        s << it;
    for(const auto & it : sets.coef)
        s << it;
    s << sets.rfactor;
    s << sets.slopeShift;
    s << sets.rangeLowLimit;
    s << sets.rangeHighLimit;
}

void um_sender_base::make_set_settings_packet(const um_gps_settings & sets)
{
    dataToSend.resize(sizeof(um_header) + sizeof(um_gps_settings));
    QDataStream s(&dataToSend, QIODevice::ReadWrite);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    s.setByteOrder(QDataStream::LittleEndian);

    s << um_cmd_group::settings;
    s << um_cmd_type::set_param;
    s << um_settings_type::gps;
    s << um_status::success;
    s << sets.baudrate;
    s << sets.startupTimeout;
    s << sets.configStringTimeout;
    for(const auto & it : sets.configString)
    {
        s << it.len;
        s.writeRawData(it.buf, um_gps_settings::config_str_max_len);
    }
}

void um_sender_base::make_get_settings_packet(um_settings_type par)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::settings ));
    dataToSend.append( static_cast<char>( um_cmd_type::get_param ));
    dataToSend.append( static_cast<char>( par                    ));
    dataToSend.append( static_cast<char>( um_status::success     ));
}

void um_sender_base::make_settings_cmd_packet(um_settings_cmd cmd, um_settings_type par)
{
    dataToSend.clear();
    dataToSend.append( static_cast<char>( um_cmd_group::settings ));
    dataToSend.append( static_cast<char>( um_cmd_type::control   ));
    dataToSend.append( static_cast<char>( cmd                    ));
    dataToSend.append( static_cast<char>( um_status::success     ));
    dataToSend.append( static_cast<char>( par                    ));
}
