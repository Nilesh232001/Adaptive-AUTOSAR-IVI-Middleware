#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "persistence.hpp"

using json = nlohmann::json;

TEST(SerializationTests, SerializeToJson) {
    // Sample data to serialize
    MyData data;
    data.field1 = "Test";
    data.field2 = 42;

    // Serialize
    json j = data.toJson();
    
    // Validate the serialized output
    EXPECT_EQ(j["field1"], "Test");
    EXPECT_EQ(j["field2"], 42);
}

TEST(SerializationTests, DeserializeFromJson) {
    // Sample JSON data
    json j = {
        {"field1", "Test"},
        {"field2", 42}
    };

    // Deserialize
    MyData data = MyData::fromJson(j);
    
    // Validate the deserialized output
    EXPECT_EQ(data.field1, "Test");
    EXPECT_EQ(data.field2, 42);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}