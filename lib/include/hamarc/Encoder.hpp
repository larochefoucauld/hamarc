#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <fstream>
#include "BitOperator.hpp"


/**
 * \brief Кодировщик. 
 * Позволяет кодировать данные, доступные посредством потока чтения, 
 * расширенным кодом Хэмминга, а также записывать код в поток вывода.
*/
class Encoder{
public:
    enum class EncodingResult{
        kSuccess,
        kReaderEOFReached,
        kReaderCorrupted
    };

/**
 * \brief Вычисляет количество контрольных бит для кодирования сообщения данного размера в коде Хэмминга
 * \attention Вычисляется размер классического, а не расширенного кода Хэмминга (без бита чётности)
 * \param msg_bit_size Размер сообщения в битах
 * \return Количество контрольных бит классического кода Хэмминга
*/
    static size_t GetCodeBitSize(size_t msg_size);

/**
 * \brief Вычисляет бит чётности сообщения
 * \param reader Поток ввода сообщения. Чтение начинается с исходной позиции.
 * \param msg_bit_size Размер сообщения в битах
 * \note По завершении перемещает позицию потока чтения на первый байт после конца сообщения
*/
    static bool GetMsgParityBit(std::istream& reader, size_t raw_msg_size);

/**
 * \brief Вычисляет бит чётности сообщения
 * \param msg Сообщение
 * \param msg_bit_size Размер сообщения в битах
*/
    static bool GetMsgParityBit(const uint8_t* msg, size_t raw_msg_size);

/**
 * \brief Вычисляет расширенный код Хэмминга для сообщения
 * \param reader Поток ввода сообщения. Чтение начинается с исходной позиции.
 * \param raw_msg_size Размер кодируемого сообщения в байтах
 * \attention Для входных данных должны выполняться гарантии:
 * из потока ввода возможно считать требуемое кол-во байт, размер сообщения ненулевой
 * \note По завершении перемещает позицию потока чтения на следующий 
 * байт после конца сообщения   
*/
    static uint8_t* GetCode(std::istream& reader, size_t raw_msg_size);

/**
 * \brief Вычисляет расширенный код Хэмминга для сообщения
 * \param msg Сообщение
 * \param raw_msg_size Размер кодируемого сообщения в байтах
 * \attention Для входных данных должны выполняться гарантии:
 * из потока ввода возможно считать требуемое кол-во байт, размер сообщения ненулевой   
*/
    static uint8_t* GetCode(const uint8_t* msg, size_t raw_msg_size);

/**
 * \brief Вычисляет расширенный код Хэмминга для сообщения и записывает его
 * \param reader Поток ввода сообщения. Чтение начинается с исходной позиции
 * \param writer Поток вывода кода
 * \param raw_msg_size Размер кодируемого сообщения в байтах
 * \note По завершении перемещает позицию потока чтения на следующий 
 * байт после конца сообщения, позицию потока вывода - на следующий байт
 * после записанного кода
 */
    static EncodingResult EncodeAndWrite(
        std::istream& reader, std::ostream& writer, size_t raw_msg_size);

/**
 * \brief Вычисляет расширенный код Хэмминга для сообщения и записывает его
 * \param msg Сообщение
 * \param writer Поток вывода кода
 * \param raw_msg_size Размер кодируемого сообщения в байтах
 */
    static EncodingResult EncodeAndWrite(
        const uint8_t* msg, std::ostream& writer, size_t raw_msg_size);

private:
    static const size_t kMaxBufferSize;

    static void ShiftAndRead(
        std::istream& reader, std::streamoff offset, 
        size_t data_size, char* buf);
        
    static bool GetByteParityBit(uint8_t byte, size_t number_of_bits);
};

#endif  // ENCODER_HPP
