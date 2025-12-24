#include "persistence.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool saveState(const std::string& filename, const json& state) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << state.dump(4); // Pretty print with 4 spaces
    file.close();
    return true;
}

json loadState(const std::string& filename) {
    std::ifstream file(filename);
    json state;

    if (file.is_open()) {
        file >> state;
        file.close();
    }

    return state;
}