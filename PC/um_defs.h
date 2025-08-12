#ifndef UM_DEFS_H
#define UM_DEFS_H

#include <stdint.h>
#include <QHostAddress>
#include <QDateTime>

struct geo_data
{
    uint16_t msec;      ///< Миллисекунды времени
    uint16_t year;      ///< Год
    uint8_t  month;     ///< Месяц
    uint8_t  day;       ///< День
    uint8_t  hour;      ///< Час
    uint8_t  min;       ///< Минута
    uint8_t  sec;       ///< Секунда
    uint8_t  satCount;  ///< Количество спутников
    bool     dataValid; ///< Данные определены
    float    latitude;  ///< Широта, градусы
    float    longitude; ///< Долгота, градусы
    float    altitude;  ///< Высота над уровнем моря, м
    float    speed;     ///< Скорость, м/с
    float    angle;     ///< Курсовой угол, градусы
    float    error;     ///< Ошибка определения положения, м
};

enum class um_event
{
    found,
    work_started,
    test_started,
    stopped,
    temp_stable,
    line_stable,
    base_written,
};

enum class um_progress { temp, line, base };

enum class um_error : uint8_t
{
    alg_temp_timeout                , ///< Таймаут стабилизации температуры при старте работы
    alg_line_timeout                , ///< Таймаут стабилзациии линии поглощения при старте работы
    alg_line_lost                   , ///< В процесс работы потеряна усточивость управления по линии поглощения
    hw_fpga_no_int_error            , ///< Аппаратный сбой: нет прерывания от FPGA
    hw_fpga_no_data_ready           , ///< Аппаратный сбой: нет бита готовности данных в статусе FPGA
    hw_fpga_ref_overflow            , ///< Аппаратный сбой: переполнение буфера реперного канала в FPGA
    hw_fpga_anl_overflow            , ///< Аппаратный сбой: переполнение буфера аналитического канала в FPGA
    hw_qspi_write_reg               , ///< Аппаратный сбой: зависание QSPI при записи регистра FPGA
    hw_qspi_read_reg                , ///< Аппаратный сбой: зависание QSPI при чтении регистра FPGA
    hw_qspi_send_cmd                , ///< Аппаратный сбой: зависание QSPI при отправке команды FPGA
    hw_qspi_read_ch                 , ///< Аппаратный сбой: зависание QSPI при чтении кнала FPGA
    hw_adc_power_up                 , ///< Аппаратный сбой: ошибка при включении АЦП
    hw_adc_calibration              , ///< Аппаратный сбой: ошибка при калибровке АЦП
    hw_adc_start                    , ///< Аппаратный сбой: ошибка при запуске преобразования АЦП
    hw_adc_stop                     , ///< Аппаратный сбой: ошибка при останове преобразования АЦП
    hw_adc_no_int                   , ///< Аппаратный сбой: нет прерывания готовности данных АЦП
    hw_laser_temp_error             , ///< Температура лазера за пределеми допустимого диапазона
    sw_alg_no_input_memory          , ///< Программный сбой: нет памяти в пуле для входных данных алгоритма
    sw_alg_no_output_memory         , ///< Программный сбой: нет памяти в пуле для выходных данных алгоритма
    sw_alg_data_lost                , ///< Программный сбой: пропущены данные алгоритма
    sw_alg_logic_error              , ///< Программный сбой: вызов функций модуля alg_module_sys_impl в неправильные моменты времени
    sw_socket_not_opened            ,
    sw_socket_not_bound             ,
    sw_no_mem_first_buffer          ,
    sw_no_mem_second_buffer         ,
    sw_socket_send_error            ,
    sw_socket_recv_error            ,
    sw_datagram_fragmented          ,
    device_not_found,
    __amount,
};

enum class um_data_flag
{
    weak_ref = 0,   ///< Флаг слабого сигнала в реперном канале
    bad_anl,        ///< Флаг «плохого» сигнала в аналитическом канале (мало импульсов прошло фильтрацию)
    over_sun,       ///< Флаг превышения порога засветки
    sign_quality,   ///< Флаг «качества» сигнала расстояния. Определяет, работает ли интеграторы величин дистанции
    leak_flag,      ///< Флаг наличия утечки определяется по порогам утечки
    wr_detr,        ///< Флаг превышения нормы концентрации
    slow_updated,   ///
};

struct um_data
{
    QDateTime dateTime;       ///< Штамп времени, проставляется при принятии по UDP пакета с данными
    // Заголовок
    uint32_t dataIndex;       ///< Время фиксации результатов расчета, UTC
    // Этап "Усреднение"
    float    amplRef;         ///< Амплитуда усредненного импульса реперного канала
    float    ampl;            ///< Усредненная амплитуда импульсов аналитического канала
    float    zero;            ///< Усредненный уровень нуля импульсов аналитического канала
    float    sun;             ///< Величина засветки в оптическом блоке
    // Этап "Температура"
    float    temperature;     ///< Температура в градусах Цельсия
    float    peltierCurrent;  ///< Ток через элемент Пельтье, мА
    float    peltierVoltage;  ///< Напряжение на элементе Пельтье, В
    float    powerVolateg;    ///< Напряжение питания прибора, В
    float    line;            ///< Положение линии провала в реперном канале, мкс
    float    peltierUset;     ///< Напряжение уставки Пельтье
    // Этап "Дистанция"
    float    Range;           ///< Промежуточный параметр – «мгновенное» значение расстояния до объекта
    float    diFast;          ///< «Быстрая» дистанция
    float    diSlow;          ///< «Медленная» дистанция
    float    fixDist;         ///< Фиксированное значение дистанции. Захватывается по команде «Запись базы»
    // Этап "Корреляция"
    float    peakRef;         ///< Положение главного пика вектора nrm_spc_flt_ref
    float    valleyRef;       ///< Положение главного провала вектора nrm_spc_flt_ref
    float    peakAnl;         ///< Положение главного пика вектора nrm_spc_flt_anl
    float    valleyAnl;       ///< Положение главного провала вектора nrm_spc_flt_anl
    float    corrPeak;        ///< Параметр корреляции положений пиков
    float    corrValley;      ///< Параметр корреляции положений провалов
    float    corrCross;       ///< Параметр корреляции пика и впадины аналитического канала
    float    corrLog;         ///< Параметр корреляции векторов nrm_spc_log_ref и nrm_spc_log_anl
    float    corrFlt;         ///< Параметр корреляции векторов nrm_spc_flt_ref и nrm_spc_flt_anl
    float    corrCoef;        ///< Коэффициент корреляции сигналов spc_flt_ref и spc_flt_anl
    float    correlation;     ///< Общий коэффициент корреляции аналитического и реперного сигналов
    // Этап "Предконцентрация"
    float    kapR;            ///< Промежуточный параметр. Рассчитывается по сигналу log_ref. Служит для расчета «опорной» концентрации метана
    float    kappaR;          ///< Промежуточный параметр. Рассчитывается по сигналу log_ref. Служит для расчета величины шума сигнала концентрации
    float    crlr;            ///< Промежуточный параметр. Определяется «опорной» концентрацией метана и параметром FactorWS. Служит для расчета «первичной» концентрации метана
    float    slope;           ///< «Коэффициент пропорциональности» аналитического и реперного каналов. Рассчитывается по сигналам spc_flt_ref и spc_flt_anl с помощью матричного фильтра Винера
    float    fastAtmCont;     ///< «Быстрая» атмосферная концентрация. Рассчитывается по расстоянию до объекта и средней концентрации метана в атмосфере Земли
    float    slowAtmCont;     ///< Медленная» атмосферная концентрация. Рассчитывается аналогично fastAtmCont
    // Этап "Быстрая концентрация"
    float    fullPrimeCont;   ///< «Полная» первичная концентрация, т.е. без вычета атмосферного метана и рассчитанная без дополнительной обработки сигнала
    float    fullNlkSN;       ///< «Полная» нормированная концентрация, измеренная при отсутствии утечки
    float    fullSN;          ///< «Полная» нормированная концентрация с вычетом fullNlkCont
    float    fullCont;        ///< «Полная» концентрация с учетом поправки на концентрацию без утечки
    float    fullNoise;       ///< Шум концентрации, рассчитывается по параметрам kappaR, sortPulses и ampl
    float    fastPrimeCont;   ///< «Быстрая» первичная концентрация, т.е. с вычетом атмосферного метана и рассчитанная без дополнительной обработки сигнала
    float    fastNlkSN;       ///< «Быстрая» нормированная концентрация, измеренная при отсутствии утечки
    float    fastSN;          ///< «Быстрая» нормированная концентрация с вычетом fastNlkCont
    float    fastCont;        ///< «Быстрая» концентрация с учетом поправки на концентрацию без утечки
    float    fastChkCont;     ///< «Быстрая» концентрация, проверенная по пороговым значениям параметров
    // Этап "Медленная концентрация"
    float    slowNlkSN;       ///< «Медленная» нормированная концентрация, измеренная при отсутствии утечки
    float    slowSN;          ///< «Медленная» нормированная концентрация с вычетом slowNlkCont
    float    slowCont;        ///< «Медленная» концентрация с учетом поправки на концентрацию без утечки
    float    slowChkCont;     ///< «Медленная» концентрация, проверенная по пороговым значениям параметров
    float    leakCont;        ///< «Концентрация утечки» – это и есть итоговая концентрация, которую рассчитывает Модуль. Формируется как минимум от slowChkCont и maxFastCont
    float    maxCorrelation;  ///< Максимальный коэффициент корреляции за медленный период
    float    meanFullCont;    ///< Средняя «полная» концентрация за медленный период
    float    meanFullSN;      ///< Средняя нормированная «полная» концентрация за медленный период
    float    meanAmpl;        ///< Средняя амплитуда аналитического сигнала за медленный период
    float    meanZero;        ///< Средний уровень нуля аналитического сигнала за медленный период
    float    maxFastCont;     ///< Максимальное замедленный период значение параметра fastCont
    // Этап "GPS"
    geo_data gpsGeoData;      ///< Геоданные по GPS
    float    gpsX;            ///< Рассчитанное по GPS расстояние X, м
    float    gpsY;            ///< Рассчитанное по GPS расстояние Y, м
    float    gpsDistance;     ///< Рассчитанное по GPS пройденное расстояние, м
    // Разные
//    uint8_t  maxIndex;        ///< Номер шага медленного цикла, на котором было зафиксировано максимальное значение fastSN
    uint16_t  sortPulses;      ///< Количество импульсов аналитического канала, прошедших фильтр
    uint16_t sumSortPulses;   ///< Сумма количества импульсов, прошедших фильтр, за медленный период
    uint32_t dataFlags;       ///< Флаги результатов расчета
    // Массив для запись в файл detr
    std::array<float, 123> leakLine;
};

enum class um_vector_id
{
    // Этап "Усреднение"
    nrm_ref,         ///< Усредненный нормированный импульс реперного канала
    nrm_anl,         ///< Усредненный нормированный импульс аналитического канала
    // Этап "Фиксация"
    fix_ref,         ///< Фиксированное значение импульса сигнала реперного канала. Захватывается по команде «Запись базы»
    fix_anl,         ///< Фиксированное значение импульса аналитического канала. Захватывается по команде «Запись базы»
    // Этап "Поглощение"
    appr_ref,        ///< Аппроксимированный сигнал реперного канала. Вычисляется путем аппроксимации параболой отношения сигнала nrmRef к сигналу fixRef с последующим умножением на fixRef
    log_ref,         ///< Логарифмированный сигнал реперного канала. Вычисляется логарифмированием отношения сигнала nrmRef к apprRef
    appr_anl,        ///< Аппроксимированный сигнал аналитического канала. Вычисляется аналогично apprRef
    log_anl,         ///< Логарифмированный сигнал аналитического канала. Вычисляется аналогично logRef
    // Этап "Спектр"
    flt_ref,         ///< Сигнал logRef, прошедший через фильтр Баттерфорта высоких частот
    flt_anl,         ///< Сигнал logAnl, прошедший через фильтр Баттерворта высоких частот
    spc_log_ref,     ///< Сигнал logRef,прошедший «спектральное преобразование», т.е. последовательно БПФ, умножение на гауссовской массив и ОБПФ
    spc_flt_ref,     ///< То же для сигнала fltRef
    spc_log_anl,     ///< То же для сигнала logAnl
    spc_flt_anl,     ///< То же для сигнала fltAnl
    nrm_spc_log_ref, ///< Сигнал spcLogRef, нормированный на свое стандартное отклонение
    nrm_spc_flt_ref, ///< Тоже для сигнала nrmSpcFltRef
    nrm_spc_log_anl, ///< Тоже для сигнала nrmSpcLogAnl
    nrm_spc_flt_anl, ///< Тоже для сигнала nrmSpcFltAnl
    // Этап "Медленная концентрация"
    line_shape,      ///< Образец сигнала аналитического канала, аппроксимированный по двум участкам вне области провала
    leak_line,       ///< Последний сохраненный сигнал lineShape, при котором было зафиксировано превышение уровня нормированной концентрации


    test_adc = 31,         ///< данные напрямую с АЦП

    __amount,
};

inline constexpr float    um_laser_min_current = 0.0;
inline constexpr float    um_laser_max_current = 300.0;
inline constexpr uint8_t  um_laser_min_time = 0;
inline constexpr uint16_t um_laser_max_time = 199;
inline constexpr float    um_laser_min_temp = -20.0;
inline constexpr float    um_laser_max_temp =  60.0;
inline constexpr float    um_laser_min_line = 0.0;
inline constexpr float    um_laser_max_line = 200.0;
inline constexpr int8_t   um_shift2_min  = -5;
inline constexpr int8_t   um_shift2_max  = 5;
inline constexpr int8_t   um_shift2_step = 1;
inline constexpr uint8_t  um_corr_shift_min  = 40;
inline constexpr uint8_t  um_corr_shift_max  = 100;
inline constexpr uint8_t  um_corr_shift_step = 1;
inline constexpr float    um_corr_pos_coef_min  = -1.0;
inline constexpr float    um_corr_pos_coef_max  = 1.0;
inline constexpr float    um_corr_pos_coef_step = 0.1;
inline constexpr uint8_t  um_corr_pos_limit_min  = 0;
inline constexpr uint8_t  um_corr_pos_limit_max  = 120;
inline constexpr uint8_t  um_corr_pos_limit_step = 1;
inline constexpr float    um_corr_delta_min  = 0;
inline constexpr float    um_corr_delta_max  = 120;
inline constexpr float    um_corr_delta_step = 0.1;
inline constexpr uint8_t  um_corr_delta_range_min  = 0;
inline constexpr uint8_t  um_corr_delta_range_max  = 120;
inline constexpr uint8_t  um_corr_delta_range_step = 1;

struct um_laser_waveform
{
    float zeroLevel;    // 0.0
    float beginLevel;   // 120
    float endLevel;     // 240
    uint16_t beginTime; // 0
    uint16_t endTime;   // 150
};

struct um_leak_level_param
{
    float corr;
    float norm;
};

enum class um_leak_level : uint8_t
{
    _1, _2, _3, _4, _5, __amount
};

inline constexpr auto um_leak_level_amount = static_cast<size_t>(um_leak_level::__amount);

struct um_temp_mode_param
{
    um_laser_waveform laserWfm;
    float workTemp;
    float workLine;
    float factorWS;
};

enum class um_temp_mode : uint8_t
{
    winter, summer, super_summer, __amount, automatic = 0xFF
};

inline constexpr auto um_temp_mode_amount = static_cast<size_t>(um_temp_mode::__amount);

enum class um_calc_mode : uint8_t
{
    fast, medium, slow, __amount,
};

inline constexpr auto um_calc_mode_amount = static_cast<size_t>(um_calc_mode::__amount);

struct um_regulator_param
{
    float    kp              ; //  0.1  ///< Пропорциональный коэффициент регулятора
    float    ki              ; //  0    ///< Интегральный коэффициент регулятора
    float    maxSetDiff      ; //  1   ///< Максимальная разница уставки и текущего значения параметра, используемой при расчете управляющего напряжения
    float    lineToTempCoef  ; //  20  ///< Коэффициент преобразования отклонения линии в отклонения температуры
    float    switchToLineThr ; //  15  ///< Значение отклонения линии, при котором происходит переключение со стабилизации по температуре на стабилизацию по линии
    float    lineStableThr   ; //  0.1 ///< Значение отклонения линии, при достижении которого считается, что стабилизация завершена
};

enum class um_pulse_filter_criteria
{
    ampl_threshold,
    ampl_relation_dir,
    ampl_relation_rev,
    dispersion,
};

struct um_algorithm_settings
{
    uint32_t pulsefilterCriteria;
    int8_t   shift2;
    uint8_t  shiftCoefA;
    uint8_t  shiftCoefB;
    uint8_t  shiftCoefC;
    int8_t   minPosCoef;
    uint8_t  minPosLowLimit;
    uint8_t  minPosHighLimit;
    int8_t   maxPosCoef;
    uint8_t  maxPosLowLimit;
    uint8_t  maxPosHighLimit;
    uint16_t delta;
    uint8_t  deltaLowLimit;
    uint8_t  deltaHighLimit;
};

enum class um_alg_state : uint8_t
{
    idle,
    temp_stabilizing,
    line_stabilizing,
    auto_test_waiting,
    auto_test,
    work,
    error,
    test_mode,
};

struct um_work_mode_settings
{
    um_leak_level_param  leakLevelParams[um_leak_level_amount];
    um_temp_mode_param   tempModeParams[um_temp_mode_amount];
    um_regulator_param   regulatorParam;
    uint16_t             tempTimeout;
    uint16_t             lineTimeout;
    uint16_t             autotestTime;
    float                refFactor;
};

enum class um_dist_array_id : uint8_t { r15, r35, coef, __amount };

inline constexpr size_t um_dist_array_size = 40;
inline constexpr size_t um_dist_coef_count = 5;

struct um_distance_settings
{
    float r15[um_dist_array_size];
    float r35[um_dist_array_size];
    float coef[um_dist_coef_count];
    float rfactor;
    float slopeShift;
    float rangeLowLimit;
    float rangeHighLimit;
};

enum class um_test_mode_control : uint8_t
{
    temperature_and_line,
    only_temperature,
    __amount,
};

struct um_test_mode_settings
{
    um_laser_waveform laserWfm;
    float workTemp; // 25
    float workLine; // 90
    um_regulator_param regParam;
    um_test_mode_control control;   // only_temperature
};

struct um_laser_settings
{
    float maxLaserCurrent;
    float maxPeltierCurrent;
    float minTemp;
    float maxTemp;
    float thermA;
    float thermB;
    float thermC;
};

struct um_link_settings
{
    QHostAddress manualIPAddres;
    QHostAddress manualSubnetMask;
    QHostAddress manualGateway;
    quint16 listenPort;
    quint16 senderPort;
    quint8  macAddr[6];
    quint32 buadrate;
    quint32 ethSendFlags;
    quint32 rs232SendFlags;
};

struct um_gps_settings
{
    static constexpr size_t config_str_max_len = 61;
    static constexpr size_t config_str_count = 4;
    struct config_str
    {
        uint8_t len;
        char buf[config_str_max_len];
    };

    uint32_t baudrate;
    uint16_t startupTimeout;
    uint16_t configStringTimeout;
    config_str configString[config_str_count];
};

#endif // UM_DEFS_H
