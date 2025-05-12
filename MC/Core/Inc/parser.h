/*
 * parser.h
 *
 *  Created on: Feb 7, 2025
 *
 */
#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdint.h>
#include "hardware.h"
#include "main.h"

#define MAX_DATA_SIZE 201 // Максимальный размер буфера.
#define SYNC_BYTE 0xAA // Синхробайт.
#define DATA_SIZE_OFFSET 3 // 2 байта crc + код команды.
#define SIZE_PAKET 7 // синхробайт + 2 байта полезных данных + cmd + status + 2 CRC.
#define CRC_INIT 0xffff // для подсчета контрольной суммы CRC.

//коды ошибок
#define STATUS_OK 0 // Ошибок нет.
#define STATUS_EXEC_ERROR 1 // Ошибка выполнения команды.
#define STATUS_INVALID_CMD 2 // Несуществующая команда.
#define STATUS_TIMED_OUT 3 // Превышено время выполнения команды.
#define STATUS_INVALID_SIZE 4 // Ошибка размера данных команды.

//Тестовые команды для отладки протокола
#define APPLY_VOLTAGE_RL1 0 // команда подать лог. 0 или 1 на RL1 для замыкания цепи питания 12В.
#define TEST_VOLTAGE_4_POINT 1 // команда проверка напр. в 4 контрольных точках +6 -6 +5 +3.3В.
#define ANALYSIS_VOLTAGE_CORRENT 2 // команда измерение напр. и тока питания.
#define APPLY_VOLTAGE_RL2 3 // Команда лог. 0 или 1 на RL2 для замыкания R1 и R22.
#define TEST_VOLTAGE_11_POINT 4 // команда проверки напр. в 12 контр. точках.
#define TEST_CORRENT_LASER 5 // Команда измерения формы тока лазерного диода.
#define TEST_VOLTAGE_PELTIE 6 // Команда измерения напряжения элемента Пельтье.
#define APPLY_VOLTAGE_5_RL 7 // команда подать лог. 0 или 1 для РАЗМЫКАНИЯ RL3-RL7.
#define MASSAGE_RS232 8 // команда отправки заготовленного пакета по RS232.
#define MASSAGE_NMEA 9 // команда отправки заготовленных пакетов NMEA на GPS через RS232.

enum parser_result {
    PARSER_OK,
    PARSER_ERROR,
    PARSER_DONE,
};

struct protocol_parser {
    enum {
        STATE_SYNC,
        STATE_SIZE_L,
        STATE_SIZE_H,
        STATE_CMD,
        STATE_DATA,
        STATE_CRC_L,
        STATE_CRC_H,
    } state;

    uint8_t buffer[MAX_DATA_SIZE];  // Буфер для хранения данных пакета.
    size_t buffer_length;         // Количество принятых байт данных.
    uint16_t data_size;             // Размер полезных данных, полученный из пакета с учетом DATA_SIZE_OFFSET.
    uint8_t cmd;                    // Команда пакета.
    uint16_t crc;                   // Накопленная контрольная сумма.
};

struct for_transfer
{
    uint8_t* buf; // Массив с пакетом байтов.
    size_t buf_size; // Размер массива buf.
    uint8_t cmd; // Код команды.
    uint8_t status; // Код ошибки.
    uint8_t* value; // Данные команды.
};

struct value_range {
    uint32_t min;
    uint32_t max;
    uint32_t voltage_4[4];
    uint32_t voltage_11[11];
};

 static const struct value_range VALUE_RANGES[] = {
   [APPLY_VOLTAGE_RL1] = {.min = 0, .max = 1},
   [TEST_VOLTAGE_4_POINT] = {.min = 1, .max = 4, .voltage_4 = {6, 3, 5, 6}},
   [ANALYSIS_VOLTAGE_CORRENT] = {.min = 0, .max = 1},
   [APPLY_VOLTAGE_RL2] = {.min = 0, .max = 1},
   [TEST_VOLTAGE_11_POINT] = {.min = 0, .max = 10, .voltage_11 = {1200, 1800, 2500, 5500, 4500, 5500, 5500, 1800, 2500, 5000, 2048}},
   [TEST_CORRENT_LASER] = {1},
   [TEST_VOLTAGE_PELTIE] = {1},
   [APPLY_VOLTAGE_5_RL] = {.min = 0, .max = 1},
   [MASSAGE_RS232] = {1},
   [MASSAGE_NMEA] =  {.min = 0, .max = 59},
};

extern struct protocol_parser parser;
extern struct for_transfer data;

enum parser_result process_rx_byte(struct protocol_parser *parser, uint8_t byte);
void serialize_reply(struct for_transfer* data);
void choose_command(uint8_t* buffer, size_t* buffer_length);// функия для выбора команды
void transmission(struct for_transfer* data, struct protocol_parser* parser); // функция для записи из for_transfer во for_receiving

#endif /* INC_PARSER_H_ */
