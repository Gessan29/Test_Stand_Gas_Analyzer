#ifndef UM_SENDER_BASE_H
#define UM_SENDER_BASE_H

#include <QObject>
#include "um_defs.h"
#include "um_protocol_defs.h"

class um_sender_base : public QObject
{
    Q_OBJECT
public:
    um_sender_base(QObject * parent = nullptr);
    ~um_sender_base();

public slots:
    // Устройство
    void get_device_param(um_dev_param par);

    // Алгоритм
    void exec_cmd(um_alg_cmd cmd);
    void exec_cmd_adc_test( um_alg_cmd_data cmd_data );
    void get_alg_param(um_alg_param par);
    void set_leak_level(um_leak_level leakLevel);
    void set_temp_mode(um_temp_mode tempMode);
    void set_calc_mode(um_calc_mode calcMode);

    // Настройки
    void set_alg_settings(std::shared_ptr<um_algorithm_settings> settings);
    void set_work_settings(std::shared_ptr<um_work_mode_settings> settings);
    void set_test_settings(std::shared_ptr<um_test_mode_settings> settings);
    void set_link_settings(std::shared_ptr<um_link_settings> settings);
    void set_laser_settings(std::shared_ptr<um_laser_settings> settings);
    void set_dist_settings(std::shared_ptr<um_distance_settings> settings);
    void set_gps_settings(std::shared_ptr<um_gps_settings> settings);

    // Чтение настроек
    void get_settings(um_settings_type type);
    void exec_settings_cmd(um_settings_cmd cmd, um_settings_type type);

protected:
    virtual void send_packet(const QByteArray & arr) = 0;

private:
    void make_dev_get_param_packet(um_dev_param par);

    void make_alg_cmd_packet(um_alg_cmd cmd);
    void make_alg_cmd_packet(um_alg_cmd cmd, um_alg_cmd_data cmd_data );
    void make_alg_set_leak_level_packet(um_leak_level leakLevel);
    void make_alg_set_temp_mode_packet(um_temp_mode tempMode);
    void make_alg_set_calc_mode_packet(um_calc_mode calcMode);
    void make_alg_get_param_packet(um_alg_param par);

    void make_set_settings_packet(const um_algorithm_settings & sets);
    void make_set_settings_packet(const um_work_mode_settings & sets);
    void make_set_settings_packet(const um_test_mode_settings & sets);
    void make_set_settings_packet(const um_link_settings & sets);
    void make_set_settings_packet(const um_laser_settings & sets);
    void make_set_settings_packet(const um_distance_settings & sets);
    void make_set_settings_packet(const um_gps_settings & sets);

    void make_get_settings_packet(um_settings_type par);
    void make_settings_cmd_packet(um_settings_cmd cmd, um_settings_type par);

private:
    QByteArray dataToSend;
};

#endif // UM_SENDER_BASE_H
