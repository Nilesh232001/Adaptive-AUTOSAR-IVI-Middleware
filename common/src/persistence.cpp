#include "persistence.hpp"
#include "logging.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>

using json = nlohmann::json;

static json load_json_file(const std::string &filename)
{
    json j;
    try {
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            return json(); // return empty
        }
        ifs >> j;
    } catch (const std::exception &e) {
        log_error(std::string("Failed to load JSON from ") + filename + ": " + e.what());
        return json();
    }
    return j;
}

static void save_json_file_atomic(const std::string &filename, const json &j)
{
    try {
        namespace fs = std::filesystem;
        fs::path target = fs::u8path(filename);
        fs::path tmp = target;
        tmp += ".tmp";

        // Write to temporary file first
        {
            std::ofstream ofs(tmp, std::ios::out | std::ios::trunc);
            if (!ofs.is_open()) {
                log_error(std::string("Failed to open temp file for writing: ") + tmp.string());
                return;
            }
            ofs << j.dump(4);
            ofs.flush();
            ofs.close();
        }

        // Atomically replace target with tmp
        std::error_code ec;
        if (fs::exists(target, ec)) {
            fs::remove(target, ec); // ignore error, will attempt rename anyway
        }
        fs::rename(tmp, target, ec);
        if (ec) {
            log_error(std::string("Failed to rename temp file to target: ") + ec.message());
            // attempt non-atomic fallback: copy & remove
            fs::copy_file(tmp, target, fs::copy_options::overwrite_existing, ec);
            if (!ec) fs::remove(tmp, ec);
            else log_error(std::string("Fallback copy failed: ") + ec.message());
        }
    } catch (const std::exception &e) {
        log_error(std::string("Failed to save JSON to ") + filename + ": " + e.what());
    }
}

// Persistence API implementations

nlohmann::json Persistence::loadConfig(const std::string& filename)
{
    return load_json_file(filename);
}

void Persistence::saveConfig(const std::string& filename, const nlohmann::json& config)
{
    save_json_file_atomic(filename, config);
}

nlohmann::json Persistence::loadState(const std::string& filename)
{
    return load_json_file(filename);
}

void Persistence::saveState(const std::string& filename, const nlohmann::json& state)
{
    save_json_file_atomic(filename, state);
}