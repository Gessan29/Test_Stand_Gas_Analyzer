#ifndef UM_PROTOCOL_DEFS_H
#define UM_PROTOCOL_DEFS_H

#include <stdint.h>

enum class um_status : uint8_t
{
    success,
    bad_state,
    bad_value,
    error,
    bad_request,
};

enum class um_cmd_group : uint8_t
{
    device    ,
    algorithm ,
    settings  ,
    gps       ,
    sd_card   ,
    invalid = 0xff
};

enum class um_cmd_type : uint8_t
{
    control,
    get_param,
    set_param,
    notification,
};

enum class um_dev_cmd : uint8_t {};

enum class um_dev_param : uint8_t { mcu_id, sw_version, hw_version };

enum class um_dev_notify : uint8_t { error };

enum class um_alg_cmd : uint8_t { start, stop, test, write_base, fix_base, test_adc };

enum class um_alg_cmd_data : uint8_t { disable = 0x00, test_adc1 = 0x01, test_adc2 = 0x02, test_adc3 = 0x03 };

enum class um_alg_param : uint8_t { state, leak_level, temp_mode, calc_mode };

enum class um_alg_notify : uint8_t
{
    result = 0x00,
    vector_nrm_ref,
    vector_nrm_anl,
    vector_fix_ref,
    vector_fix_anl,
    vector_appr_ref,
    vector_log_ref,
    vector_appr_anl,
    vector_log_anl,
    vector_flt_ref,
    vector_flt_anl,
    vector_spc_log_ref,
    vector_spc_flt_ref,
    vector_spc_log_anl,
    vector_spc_flt_anl,
    vector_nrm_spc_log_ref,
    vector_nrm_spc_flt_ref,
    vector_nrm_spc_log_anl,
    vector_nrm_spc_flt_anl,
    vector_line_shape,
    vector_leak_line,
    event,
    temp_progress,
    line_progress,
    base_progress,
    vector_test_adc = 32,
};

enum class um_alg_event : uint8_t
{
    temp_stable,
    line_stable,
    base_written,
};

enum class um_settings_cmd : uint8_t { save, restore, to_factory };

enum class um_settings_type : uint8_t
{
    algorithm_settings,
    work_mode_settings,
    test_mode_settings,
    link_settings,
    distance,
    laser,
    gps
};

struct um_header
{
    um_cmd_group cmdGroup = um_cmd_group::invalid;
    um_cmd_type  cmdType;
    union
    {
        um_dev_param     devParam;
        um_dev_notify    devNotify;
        um_alg_cmd       algCtrlCmd;
        um_alg_param     algParam;
        um_alg_notify    algNotify;
        um_settings_cmd  settingsCmd;
        um_settings_type settingsType;
    };
    um_status status;

    static um_header from_raw(const uint8_t * data)
    {
        return um_header
        {
            static_cast<um_cmd_group>(data[0]),
            static_cast<um_cmd_type>(data[1]),
            { static_cast<um_dev_param>(data[2]) },
            static_cast<um_status>(data[3])
        };
    }

    uint8_t * buf() { return reinterpret_cast<uint8_t*>(this); }
    const uint8_t * buf() const { return reinterpret_cast<const uint8_t*>(this); }
    static constexpr size_t len() { return sizeof(um_header); }
};


#endif // UM_PROTOCOL_DEFS_H
