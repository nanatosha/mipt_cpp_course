// Каркас агента: читает журнал событий построчно и считает строки.
//
// Это заготовка занятия 1.1, а не решение. Детектов она не ищет — их вы
// добавите здесь же, в отмеченном месте ниже. Формат строки детекта, список
// признаков и правило про их порядок заданы в постановке занятия: по ним
// сравниваются эталоны.
//
// Весь код лежит в main, и на этом занятии так и надо: функции появятся
// на занятии 1.2, ссылки — на 1.3. Разбор аргументов, коды возврата и флаг
// --quiet — часть задания.
//
// Запуск:
//   nano-edr <журнал.log>
#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include <map>


//детекты
const std::vector<std::string> signatures = {
        "wscript.exe", ".locked", "certutil.exe", "\\Startup\\"
    };

int main(int argc, char** argv) {


    //проверка на аргументы
    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }


    //проверка файла
    std::ifstream log(argv[1]);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return 2;
    }


    //проверка на квайт
    bool quiet = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
        quiet = true;
        }
    }



    //переменные
    long long lines = 0;
    long long comments = 0;
    std::string line;
    long long total = 0;
    std::map<std::string, long long> type_counts;



    //чтение строк файла
    while (std::getline(log, line)) {
        
        ++lines;


        size_t k = 0;
        while (k < line.size() && (line[k] == ' ' || line[k] == '\t')){
            k++;
        }

        if (k == line.size() || line[k] == '#' || line[k] == ';'){
            ++comments;
            continue;
        }


        total++;

        for (size_t i = 0; i < signatures.size(); ++i) {
            if (line.find(signatures[i]) != std::string::npos){
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, signatures[i], line);
            }
        }


        size_t type_pos = line.find("type=");
        if (type_pos != std::string::npos){
            size_t start = type_pos + 5;
            size_t end = line.find(' ', start);
            std::string type_val;
            if (end == std::string::npos) {
                type_val = line.substr(start);
            } 
            else {
                type_val = line.substr(start, end - start);
            }
            type_counts[type_val]++;
        }


    }

    if (!quiet) {
     std::print("строк {}, из них комментариев {}\n", lines, comments);
     std::print("всего событий: {}\n", total);
     std::print("типы событий:\n");
     for (const auto& [type, count] : type_counts) {
         std::print("  {}: {}\n", type, count);
     }
 }
    return 0;

}
