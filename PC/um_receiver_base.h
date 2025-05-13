#ifndef UM_RECEIVER_BASE_H
#define UM_RECEIVER_BASE_H

#include "um_defs.h"
#include "um_protocol_defs.h"
#include "data_logger_defs.h"
#include <QObject>

class um_receiver_base: public QObject
{
    Q_OBJECT
protected:
    um_receiver_base(QObject * parent = nullptr);
    ~um_receiver_base();

signals:
    // Устройство
    void got_mcu_id(QByteArray arr);
    void got_sw_version(quint8 major, quint8 minor);
    void got_hw_version(quint8 major, quint8 minor);
    void error_occured(um_error error);

    // Алгоритм
    void alg_cmd_executed(um_alg_cmd cmd, um_status status);
    void alg_param_set(um_alg_param par, um_status status);
    void got_leak_level(um_leak_level leakLevel);
    void got_temp_mode(um_temp_mode leakLevel);
    void got_calc_mode(um_calc_mode calcLevel);
    void got_alg_state(um_alg_state algState);
    void event_occured(um_event event);
    void progress_updated(um_progress progress, int pct);
    void data_ready(std::shared_ptr<um_data> data);
    void vector_received(um_vector_id id, std::vector<float> vector);

    // Настройки
    void settings_set(um_status status, um_settings_type type);
    void settings_cmd_executed(um_status status, um_settings_type type, um_settings_cmd cmd);
    void got_alg_settings(std::shared_ptr<um_algorithm_settings> sets);
    void got_work_settings(std::shared_ptr<um_work_mode_settings> sets);
    void got_test_settings(std::shared_ptr<um_test_mode_settings> sets);
    void got_link_settings(std::shared_ptr<um_link_settings> sets);
    void got_laser_settings(std::shared_ptr<um_laser_settings> sets);
    void got_dist_settings(std::shared_ptr<um_distance_settings> sets);
    void got_gps_settings(std::shared_ptr<um_gps_settings> sets);

    // Данные для логирования
    void add_fast_record(std::shared_ptr<data_logger_fast_record> rec);
    void add_slow_record(std::shared_ptr<data_logger_slow_record> rec);
    void add_detr_record(std::shared_ptr<data_logger_detr_record> rec);
    void add_error_record(std::shared_ptr<data_logger_error_record> rec);

protected:
    void decode_packet(const QByteArray & arr);

private:
    void decode_as_cmd_from_device_group(um_header hdr, QDataStream & s);
    void decode_mcu_id(QDataStream & s);
    void decode_sw_version(QDataStream & s);
    void decode_hw_version(QDataStream & s);
    void decode_as_get_mcu_id_answer();

    void decode_as_cmd_from_alg_group(um_header hdr, QDataStream & s);
    void decode_alg_event(QDataStream & s);
    void decode_progress(um_progress pr, QDataStream & s);
    void decode_alg_result(QDataStream & s);
    void decode_alg_vector(um_alg_notify nt, QDataStream & s);

    void decode_as_cmd_from_settings_group(um_header hdr, QDataStream & s);
    void decode_alg_settings(QDataStream & s);
    void decode_work_settings(QDataStream & s);
    void decode_test_settings(QDataStream & s);
    void decode_link_settings(QDataStream & s);
    void decode_laser_settings(QDataStream & s);
    void decode_dist_settings(QDataStream & s);
    void decode_gps_settings(QDataStream & s);

    template<class T> std::shared_ptr<T> make_record(const um_data & ref);

    data_logger_fast_record make_fast_record(const um_data & ref);
    data_logger_slow_record make_slow_record(const um_data & ref);
    data_logger_detr_record make_detr_record(const um_data & ref);
};

#endif // UM_RECEIVER_BASE_H
