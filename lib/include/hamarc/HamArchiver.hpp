/*
Формат .haf
- Архив всегда не пуст (содержит хотя бы один файл)
- Для каждого файла хранятся метаданные:
    <размер названия (байты)><размер содержимого (байты)>
    <размер кодируемого блока (байты)><контроль><название><контроль>
    Размеры указываются в байтах и занимают соответственно 4, 8 и 8 байт
- Файлы храняться друг за другом непрерывно в формате:
    <метаданные, контроль><содержимое><контроль содержимого>
*/

#ifndef HAMARCHIVER_HPP
#define HAMARCHIVER_HPP

#include "Encoder.hpp"
#include "Decoder.hpp"
#include "FileOperator.hpp"
#include <vector>

class HamArchiver{
public:
    HamArchiver();
    HamArchiver(std::filesystem::path working_dir);
    void SetDir(std::filesystem::path new_dir);

    struct FileMetadata {
        std::filesystem::path path;
        size_t size;
        size_t encoding_block_size;
    };

    enum class CreationResult {
        kSuccess,
        kArcAlreadyExists,
        kEmptyFileList,
        kFileNotFound,
        kFileNotAccessible
    };

    enum class ExtractionResult {
        kSuccess,
        kArcNotFound,
        kArcCorrupted,
        kEmptyFileList,
        kFileNotFound,
        kFileCorrupted
    };


    enum class AdditionResult {
        kSuccess,
        kArcNotFound,
        kEmptyFileList,
        kFileNotFound,
        kFileNotAccessible
    };

    enum class ConcatenationResult {
        kSuccess,
        kArcAlreadyExists,
        kEmptyFileList,
        kFileNotFound
    };

    std::vector<CreationResult> Create(std::string_view arcname, 
        const std::vector<FileMetadata>& files);

    std::vector<FileMetadata> GetFileList(std::filesystem::path arcfile);

    std::vector<ExtractionResult> ExtractFiles(std::filesystem::path arcfile, 
        const std::vector<std::string>& filenames);
    
    std::vector<ExtractionResult> ExtractFiles(std::filesystem::path arcfile);

    std::vector<ExtractionResult> DeleteFiles(std::filesystem::path arcfile, 
        const std::vector<std::string>& filenames);

    std::vector<AdditionResult> AppendFiles(std::filesystem::path arcfile, 
        const std::vector<FileMetadata>& files);
    
    std::vector<ConcatenationResult> Merge(std::string_view arcname,
        const std::vector<std::string>& arcfiles);

private:
    static const size_t kNumericMetadataSize;

    FileOperator file_operator;

/**
 * \brief Перезаписывает архивный файл, исключая набор файлов
 * \param arcfile Путь к архивному файлу
 * \param skip_list Названия файлов в архиве, которые будут пропущены при перезаписи
 * \param extract Флаг извлечения пропускаемых файлов
 * \attention Требуется, чтобы архивный файл существовал в рабочей директории
 * \note Файлы с корректными числовыми метаданными, идентификация которых невозможна (имя файла повреждено), 
 * остаются в архиве
*/
    std::vector<ExtractionResult> RebuildArc(std::filesystem::path arcfile,
    const std::vector<std::string>& skip_list, bool extract);

/**
 * \brief Восстанавливает декодированный файл из архива
 * \param metadata Предварительно извлечённые метаданные файла
 * \param stream Поток чтения и записи архива
 * \param forced Флаг извлечения файла при необратимом повреждении
 * \attention Требуется, чтобы 
 * 1) Было возможно создать файл с данным названием в рабочей директории
 * 2) Метаданные файла были извлечены и проверены заранее.
 * 3) Начальная позиция потока была установлена на начало первого блока
 * закодированного содержимого файла.
 * \note По завершении перемещает позицию потока на первый байт после конца данных файла
*/
    ExtractionResult ExtractFile(FileMetadata metadata, std::fstream& stream, bool forced);

/**
 * \brief Записывает закодированные метаданные в поток вывода
 * \param file Информация о файле: путь, размер, длина кодируемого блока
 * \param writer Поток записи метаданных
*/
    void WriteEncodedMetadata(FileMetadata file, std::ostream& writer/*, std::istream& reader*/);

/**
 * \brief Открывает файл (либо сообщает о невозможности это сделать), 
 * кодирует его с данной длиной блока и выводит в поток
 * \param file Информация о файле: путь, ожидаемая длина кодируемого блока
 * \param writer Поток записи закодированного файла
*/
    AdditionResult WriteEncodedFile(FileMetadata file, std::ostream& writer/*, std::istream& reader*/);

/**
 * \brief Получает метаданные файла из потока и проводит их валидацию
 * \param stream Поток чтения и записи
 * \attention В случае невалидности числовых метаданных, размер файла и кодирующего блока выставляются равными -1.
 * В случае невалидности имени файла, его считывание не производится.
 * \note По завершении позиция потока чтения устанавливается на первый байт после
 * контроля метаданных (первый байт содержимого файла)
*/
    FileMetadata GetMetadata(std::fstream& stream);

/**
 * \brief Вычисляет размер сообщения, закодированного блоками 
 * при помощи расширенного кода Хэмминга (исходный размер + кол-во дополнительных байт)
 * \param raw_msg_size Размер незакодированного сообщения (в байтах)
 * \param encoding_block_size Размер кодируемых блоков (в байтах)
*/
    size_t GetEncodedMsgSize(size_t file_size, size_t encoding_block_size);

/**
 * \brief Вычисляет размер сообщения, закодированного одним блоком
 * при помощи расширенного кода Хэмминга (исходный размер + кол-во дополнительных байт)
 * \param raw_msg_size Размер незакодированного сообщения (в байтах)
*/
    size_t GetEncodedMsgSize(size_t raw_msg_size);

/**
 * \brief Вычисляет размер кода сообщения, закодированного одним блоком 
 * при помощи расширенного кода Хэмминга (кол-во дополнительных байт)
 * \param raw_msg_size Размер незакодированного сообщения (в байтах)
*/
    size_t GetMsgCodeSize(size_t raw_msg_size);
};

#endif  // HAMARCHIVER_HPP
