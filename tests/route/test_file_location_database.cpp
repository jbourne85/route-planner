#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "route/FileLocationDatabase.h"

using namespace route;

class MockFileLocationDatabase : public FileLocationDatabase {
public:
    MockFileLocationDatabase(const std::string location_file) : FileLocationDatabase(location_file) {}
    
    MOCK_METHOD(std::vector<const Location* const>, GetLocationsOnDisk, (), (override));
    
    std::vector<const Location* const> RealGetLocationsOnDisk() {
        return FileLocationDatabase::GetLocationsOnDisk();
    }

    const Location* const GetLocationByName(std::vector<const Location* const> locations, std::string name) {
        std::vector<const Location* const> results;
        std::for_each (locations.begin(), locations.end(), [this, &results, name](const Location* const location) mutable {
            if (location->Name() == name) {
                results.push_back(location);
            }
            
        });
        if (1 == results.size()) {
            return results[0];
        }
        return nullptr;
    }
    
};

TEST(AddTest, TestLoadSuccess)
{
    const std::string test_data_file = "test_data/route/test_load_success.csv";
    MockFileLocationDatabase test_db(test_data_file);

    auto locations = test_db.RealGetLocationsOnDisk();
    
    EXPECT_EQ(3, locations.size());

    // Check the 3 cities are loaded and have correct info
    auto london = test_db.GetLocationByName(locations, "London");

    EXPECT_NE(nullptr, london);
    EXPECT_EQ("London", london->Name());
    EXPECT_EQ(5, london->Cost());

    auto glasgow = test_db.GetLocationByName(locations, "Glasgow");

    EXPECT_NE(nullptr, glasgow);
    EXPECT_EQ("Glasgow", glasgow->Name());
    EXPECT_EQ(3, glasgow->Cost());

    auto brighton = test_db.GetLocationByName(locations, "Brighton");

    EXPECT_NE(nullptr, brighton);
    EXPECT_EQ("Brighton", brighton->Name());
    EXPECT_EQ(1, brighton->Cost());    
}

TEST(AddTest, TestLoadErrorFileMissing)
{
    const std::string test_data_file = "test_data/route/test_non_existant_file.csv";
    MockFileLocationDatabase test_db(test_data_file);

    auto locations = test_db.RealGetLocationsOnDisk();

    EXPECT_EQ(0, locations.size());
}

TEST(AddTest, TestLoadErrorWhileParsing)
{
    const std::string test_data_file = "test_data/route/test_load_bad_data.csv";
    MockFileLocationDatabase test_db(test_data_file);

    auto locations = test_db.RealGetLocationsOnDisk();

    EXPECT_EQ(0, locations.size());
}