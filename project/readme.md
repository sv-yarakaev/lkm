---
# yaml-language-server: $schema=schemas\page.schema.json
Object type:
    - Page
Creation date: "2025-10-13T19:00:24Z"
Created by:
    - Stanislav
id: bafyreif46u2cdxyqr3k4podtdxkt5shl6fjfdt5fdef6sv4pxaw5za6poq
---
# readme.md   
RAM Disk Driver (bdram) блочное уcтройство   
Этот драйвер реализует виртуальные блочные устройства в оперативной памяти (/dev/bram0, /dev/bram1, ...) с расширенной статистикой, поддержкой ioctl, sysfs и возможностью дампа содержимого.   
Он основан на оригинальном brd.c из ядра Linux, но дополнен диагностическими и управляющими функциями.   
Также из оргинального файла блочного устройства были удалены функции [brd\_check\_and\_reset\_par](https://elixir.bootlin.com/linux/v6.1.130/C/ident/brd_check_and_reset_par)() [brd\_probe](https://elixir.bootlin.com/linux/v6.1.130/C/ident/brd_probe)([dev\_t](https://elixir.bootlin.com/linux/v6.1.130/C/ident/dev_t)dev), [ramdisk\_size](https://elixir.bootlin.com/linux/v6.1.130/C/ident/ramdisk_size)(char\*[str](https://elixir.bootlin.com/linux/v6.1.130/C/ident/str)). [\_\_register\_blkdev](https://elixir.bootlin.com/linux/v6.1.130/C/ident/__register_blkdev)  была заменена на register\_bllkdev.     
   
## Сборка   
Требуется исходный код ядра Linux (версия **6.1.130** или совместимая). Также собиралась по Fedora c пакетом заголовочных файлов 6.1.1-200.  В проекте присутствует Makefile который осуществляет сборку. На выходе получаем файл **bdram.ko**   
   
## Загрузка и создание дисков   
Загрузите модуль с параметрами:   
```
# Пример: 2 диска по 32 MiB каждый
sudo insmod bdram.ko rd_nr=2 rd_size=32768

```
Параметры:   
- `rd\_nr` — количество дисков (по умолчанию: 3)   
- `rd\_size` — размер одного диска в **килобайтах** (по умолчанию: 100 MiB)   
   
После загрузки появятся устройства:   
```
ls /dev/bram*
# /dev/bram0  /dev/bram1  ...

```
*Примечание: bramX были названы, когда одновременно сравнивал оригигнал и модифицированную версию.*   
   
## Общая статистика через proc   
Общую статистику можно посмотреть в каталоге /proc/bramdev/  в файле stat    
```
stv@deb:~/lkm/project/src$ ls /proc/bramdev/stat 
/proc/bramdev/stat
stv@deb:~/lkm/project/src$ cat  /proc/bramdev/stat 
Dev    Size(KB)   Reads      Writes     ReadSec      WriteSec     MultiSeg   SmallReq   Errors     AvgSize(B)  
ram0    104860     6          0          24           0            0          6          0          2048        
ram1    104860     6          0          24           0            0          6          0          2048        
ram2    104860     6          0          24           0            0          6          0          2048        
stv@deb:~/lkm/project/src$ 

```
   
   
##  Статистика через sysfs   
Все метрики доступны в подкаталоге `ramstat`:   
```
# Пример для bram0
ls /sys/block/bram0/ramstat/
# reads  writes  read_sectors  written_sectors  errors  multi_segment_bios  small_requests  size_bytes  uptime_ms  reset_stats

```
### Чтение статистики:   
```
cat /sys/block/bram0/ramstat/reads
cat /sys/block/bram0/ramstat/uptime_ms

```
### Сброс счётчиков:   
```
echo 1 | sudo tee /sys/block/bram0/ramstat/reset_stats

```
 --- 
##  Управление через ioctl   
Для продвинутых операций используйте утилиту `bd\_test` (см. ниже).   
Поддерживаемые команды:   
|                Команда |                                                   Описание |
|:-----------------------|:-----------------------------------------------------------|
| `BRD\_GET\_UPTIME\_MS` |                            Получить время жизни диска (мс) |
|    `BRD\_RESET\_STATS` |                                        Сбросить статистику |
|            `BRD\_DUMP` |                            Прочитать секторы диска в буфер |

 --- 
##  Тестирование: утилита bd\_test   
Скомпилируйте тестовую программу:   
```
gcc -o bd_test bd_test.c

```
### Использование:   
```
# Показать время жизни
sudo ./bd_test /dev/bram0

# Сбросить статистику
sudo ./bd_test /dev/bram0 reset

# Дамп 8 секторов (4 КБ) в файл
sudo ./bd_test /dev/bram0 dump dump.bin 0 8

# Просмотреть дамп
hexdump -C dump.bin | head

```
>  Даже если диск "пуст", дамп вернёт нули (ленивая аллокация с нулевой инициализацией).   

 --- 
## 🧹 Очистка   
Перед выгрузкой убедитесь, что диски не используются:   
```
sudo umount /dev/bram0  # если был смонтирован
sudo rmmod bdram

```
Все данные **автоматически удаляются** из памяти.   
 --- 
   
