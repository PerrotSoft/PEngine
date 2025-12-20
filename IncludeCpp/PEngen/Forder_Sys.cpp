#include "../../Include/PEngen/Forder_Sys.h"
#include <iostream>
#include <fstream>
#include <sstream>

// ==============================================================================
// 2. РЕАЛИЗАЦИЯ ВСПОМОГАТЕЛЬНЫХ ФУНКЦИЙ
// ==============================================================================

bool PEexists_file(const std::string& path) {
    std::error_code ec;
    return fs::is_regular_file(path, ec);
}

bool PEexists_folder(const std::string& path) {
    std::error_code ec;
    return fs::is_directory(path, ec);
}

bool PEcreate_folder(const std::string& path) {
    if (!PEexists_folder(path)) {
        std::error_code ec;
        // Используем create_directories, чтобы создать весь путь
        if (fs::create_directories(path, ec)) {
            PElogger((std::string("Folder created: ") + path).c_str());
            return true;
        }
        else {
            PElogger((std::string("ERROR: Failed to create folder: ") + path + " | Reason: " + ec.message()).c_str());
            return false;
        }
    }
    return true;
}

// ==============================================================================
// 3. ФУНКЦИЯ РАСПАКОВКИ (Место для вашего кода cpp-zipper)
// ==============================================================================

/**
 * @brief Распаковывает ZIP-архив в целевой каталог (РЕАЛИЗАЦИЯ).
 * @note Этот блок должен быть заменен на реальный код cpp-zipper/MiniZip.
 * @param zipPath Путь к ZIP-файлу.
 * @param destinationPath Целевой путь для распаковки (BASE_PATH).
 * @return true, если распаковка успешна.
 */
bool PEextract_zip(const std::string& zipPath, const std::string& destinationPath) {
    PElogger((std::string("Attempting to extract: ") + zipPath + " to " + destinationPath).c_str());

    // ---------------------------------------------------------------------------------
    // >>> ВСТАВЬТЕ СЮДА КОД cpp-zipper/MiniZip <<<
    // ---------------------------------------------------------------------------------

    // ВРЕМЕННЫЙ КОД: Запускает создание папок для имитации успешного восстановления, 
    // но в реальном проекте должен быть код распаковки!

    // Если реальный код распаковки отсутствует, мы завершаем программу с ошибкой.
    // Замените этот throw на код, который возвращает false при неудаче распаковки.

    // ВРЕМЕННАЯ ИМИТАЦИЯ УСПЕХА ДЛЯ ПРОДОЛЖЕНИЯ ЛОГИКИ:
    PElogger("ZIP library code is missing. SIMULATING SUCCESS.");
    // Имитируем, что распаковка успешно создала критические папки:
    if (!PEcreate_folder(CFG_PATH) || !PEcreate_folder(DATA_PATH) || !PEcreate_folder(LOG_PATH)) {
        return false; // Имитация сбоя при создании базовых папок
    }
    return true;
}

// ==============================================================================
// 4. РЕАЛИЗАЦИЯ ОСНОВНЫХ ФУНКЦИЙ СИСТЕМЫ ПАПОК
// ==============================================================================

bool PEInitFolderStructure() {
    PElogger("Initializing PEngen Folder Structure...");

    std::vector<std::string> folders = {
        CFG_PATH, DATA_PATH, LOG_PATH, MODELS_PATH, SHADERS_PATH, TEXTURES_PATH,
        DATA_ICONS_PATH, DATA_RE_PATH
    };

    bool all_ok = true;
    for (const auto& folder : folders) {
        if (!PEcreate_folder(folder)) {
            all_ok = false;
        }
    }

    if (!all_ok) {
        PElogger("CRITICAL: Failed to create some folders. Restoration will be attempted.");
    }

    return all_ok;
}

bool PERestoreStructure() {
    PElogger("Starting Integrity Check...");

    std::vector<std::string> critical_paths = {
        CFG_PATH, DATA_PATH, LOG_PATH,
        RECOVERY_FILE_PATH
    };

    bool integrity_ok = true;

    // Проверка целостности
    for (const auto& path : critical_paths) {
        if (!PEexists_folder(path) && !PEexists_file(path)) {
            PElogger((std::string("MISSING CRITICAL RESOURCE: ") + path).c_str());
            integrity_ok = false;
            break;
        }
    }

    if (integrity_ok) {
        PElogger("Integrity check passed. Structure is intact.");
        return true;
    }

    // Если целостность нарушена, пытаемся восстановить
    PElogger("CRITICAL: Structure integrity FAILED! Attempting restoration...");

    if (!PEexists_file(RECOVERY_FILE_PATH)) {
        PElogger((std::string("FATAL: Recovery file not found: ") + RECOVERY_FILE_PATH).c_str());
        PElogger("Cannot restore project structure.");
        return false;
    }

    PElogger("Recovery file found. Starting ZIP extraction...");

    if (PEextract_zip(RECOVERY_FILE_PATH, BASE_PATH)) {
        PElogger("Structure successfully restored from recovery file.");
        return true;
    }
    else {
        PElogger("FATAL: Failed to extract recovery file. Recovery FAILED.");
        return false;
    }
}

void PEinit_struct() {
    // 1. Пытаемся создать все папки.
    PEInitFolderStructure();

    // 2. Всегда вызываем RestoreStructure, чтобы проверить целостность
    if (!PERestoreStructure()) {
        // Если RestoreStructure вернул false (структура повреждена и восстановление не удалось)
        throw std::runtime_error("FATAL: Project structure could not be verified or restored. Exiting.");
    }

    PElogger("Project structure initialized and verified successfully.");
}