#ifndef DECODER_HPP
#define DECODER_HPP

#include <fstream>
#include <cstdint>

/**
 * \brief Декодировщик.
 * Обнаруживает ошибки в сообщении, закодированном расширенным кодом Хэмминга. 
 * Поддерживает исправление единичной ошибки "на месте", а также обнаружение
 * двойной ошибки.
*/
class Decoder{
public:
    enum class ValidationResult{
        kValid,
        kSingleErrorFixed,
        kDoubleError
    };

/**
 * \brief Проверяет наличие и исправляет ошибки в сообщении, 
 * закодированном с помощью расширенного кода Хэмминга.
 * \param msg Поток чтения и записи для проверки и исправления сообщения. 
 * Чтение начинается с исходной позиции
 * \param raw_msg_size Размер информационной части сообщения (в байтах)
 * \attention Для входных данных должны выполняться гарантии:
 * сообщение не пусто, код сообщения идёт после него и имеет корректный размер
 * \note По завершении позиция потока перемещается в начало сообщения. В случае
 * исправления ошибки буфер потока очищается.
*/
    static ValidationResult Validate(std::fstream& msg, size_t raw_msg_size);

private:
    static void FixBit(std::fstream& msg, std::streamoff error_bit_pos);
};

#endif  // DECODER_HPP
