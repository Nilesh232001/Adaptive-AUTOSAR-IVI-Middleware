#include <nlohmann/json.hpp>
#include "persistence.hpp"
#include "logging.hpp"
#include <iostream>

using json = nlohmann::json;

// Simple test for JSON serialization/deserialization
class SimpleData {
public:
    std::string field1;
    int field2;

    json toJson() const {
        json j;
        j["field1"] = field1;
        j["field2"] = field2;
        return j;
    }

    static SimpleData fromJson(const json& j) {
        SimpleData data;
        data.field1 = j.value("field1", std::string(""));
        data.field2 = j.value("field2", 0);
        return data;
    }
};

int main() {
    log_info("Starting Serialization Tests");

    try {
        // Test 1: Serialize to JSON
        {
            SimpleData data;
            data.field1 = "Test";
            data.field2 = 42;

            json j = data.toJson();
            
            if (j["field1"] != "Test" || j["field2"] != 42) {
                log_error("Serialization test FAILED");
                return 1;
            }
            log_info("Serialization test PASSED");
        }

        // Test 2: Deserialize from JSON
        {
            json j = {
                {"field1", "Test"},
                {"field2", 42}
            };

            SimpleData data = SimpleData::fromJson(j);
            
            if (data.field1 != "Test" || data.field2 != 42) {
                log_error("Deserialization test FAILED");
                return 1;
            }
            log_info("Deserialization test PASSED");
        }

        log_info("All tests completed successfully");
        return 0;

    } catch (const std::exception& e) {
        log_error(std::string("Test exception: ") + e.what());
        return 1;
    }
}