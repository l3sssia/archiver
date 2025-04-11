[English Description](#english-description) | [Описание на русском языке](#russian-description)

# Hamming Archiver

# <a id="english-description">ITMO Laboratory Work</a>

## Task

Implement an error-resistant file archiver without compression, **HamArc**, that combines multiple files into a single archive. Use [Hamming Codes](https://en.wikipedia.org/wiki/Hamming_code) for error-resistant encoding. The data storage format in the archive is also part of the task.

## Requirements

* Combine multiple files (>= 1) into a single archive in the `.haf` format (Hamming Archive File)
* Extract all or specific files from the archive
* Merge multiple archives into one
* Recover the archive in case of damage, or report if recovery is impossible
* Return a list of files in the archive

## Implementation

A console application supporting the following command-line arguments:

**-c, --create**           - create a new archive

**-f, --file=[ARHCNAME]**  - name of the archive file

**-l, --list**             - display the list of files in the archive

**-x, --extract**          - extract files from the archive (if not specified, all files)

**-a, --append**           - add a file to the archive

**-d, --delete**           - delete a file from the archive

**-A, --concatenate**      - merge two archives

**File names are passed as free arguments**

**Arguments for encoding and decoding are also passed via the command line** (The names and types of arguments are part of the task)

### Examples of execution

*hamming_code --create --file=ARCHIVE FILE1 FILE2 FILE3*

*hamming_code -l -f ARCHIVE*

*hamming_code --concatenate ARCHIVE1 ARCHIVE2 -f ARCHIVE3*

## NB

- Files for archiving can be very large
- Operations must work optimally (this affects the final grade for the work)
- The code must be structured and logically divided into modules (this affects the final grade for the work)
- Think about how to decompose the task
- Using tests will be an advantage

# <a id="russian-description">ИТМО Лабораторная работа</a>

## Задача

Реализовать помехоустойчивый архиватор файлов без сжатия **HamArc**, объединяющий несколько файлов в один архив. Для помехоустойчивого кодирования использовать [Коды Хэмминга](https://en.wikipedia.org/wiki/Hamming_code). Формат хранения данных в архиве также является частью задачи.

## Требования

* Объединять несколько файлов ( >= 1) в один архив в формате .haf (Hamming Archive File)
* Извлекать все или отдельные файлы из архива
* Объединять несколько архивов в один
* Восстанавливать архив при повреждениях, либо сообщать о том что это невозможно
* Возвращать список файлов в архиве

## Реализация

Консольное приложение, поддерживающее следующие аргументы командной строки:

**-c, --create**           - создание нового архива

**-f, --file=[ARHCNAME]**  - имя файла с архивом

**-l, --list**             - вывести список файлов в архиве

**-x, --extract**          - извлечь файлы из архива  (если не указано, то все файлы)

**-a, --append**           - добавить файл в архив

**-d, --delete**           - удалить файл из архива

**-A, --concatenate**      - смерджить два архива

**Имена файлов передаются свободными аргументами**

**Аргументы для кодирования и декодирования так же передаются через командую строку** (Названия и типы аргументов часть задания)

### Примеры запуска

*hamming_code --create --file=ARCHIVE FILE1 FILE2 FILE3*

*hamming_code -l -f ARCHIVE*

*hamming_code --concantenate  ARCHIVE1 ARCHIVE2 -f ARCHIVE3*


## NB

- Файлы для архивации могут оказаться очень большими
- Операции должны работать оптимально (от этого зависит итоговый балл за работу)
- Код должен быть структурирован и логично разбит на модули (от этого зависит итоговый балл за работу)
- Подумайте как можно декомпозировать задачу
- Использование тестов будет являться приимуществом

