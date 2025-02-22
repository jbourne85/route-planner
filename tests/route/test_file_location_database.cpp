#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include "route/Location.h"
#include "route/FileLocationDatabase.h"

using namespace route;

const std::string TEST_FILE_LOCATION_DATBASE_DATA_DIR("test_data/route/test_file_location_database");

/// @brief this is the Mock File Database class, its used to mock and control the locations read from disk
class MockFileLocationDatabase : public FileLocationDatabase {
public:
    MockFileLocationDatabase() : FileLocationDatabase("test_data.csv") {}
    MockFileLocationDatabase(const std::string location_file) : FileLocationDatabase(location_file) {}
    
    MOCK_METHOD(void, DeleteLocations, (std::vector<Location*>&), (override));
    MOCK_METHOD(std::vector<Location*>, GetLocationsOnDisk, (), (override));
    MOCK_METHOD(void, AddLocation, (Location* const), (override));

    /// @brief Adapter method to call FileLocationDatabase::GetLocationsOnDisk from the mock
    const std::vector<Location*> RealGetLocationsOnDisk() {
        return FileLocationDatabase::GetLocationsOnDisk();
    }

    /// @brief Adapter method to call FileLocationDatabase::DeleteLocations from the mock
    void RealDeleteLocations(std::vector<Location*>& locations) {
        FileLocationDatabase::DeleteLocations(locations);
    }

    /// @brief Adapter method to call FileLocationDatabase::AddLocation from the mock
    void RealAddLocation(Location* location) {
        FileLocationDatabase::AddLocation(location);
    }

    /// @brief Utility method to return a Location by name, useful for simplifying the test code
    /// @param locations the list of locations to search
    /// @param name The name of the location to find
    /// @return The location on success, nullptr on failure
    Location* const MockGetLocationByName(const std::vector<Location*> locations, std::string name) {
        std::vector<Location* const> results;
        std::for_each (locations.begin(), locations.end(), [this, &results, name](Location* const location) mutable {
            if (location->Name() == name) {
                results.push_back(location);
            }
            
        });
        if (1 == results.size()) {
            return results[0];
        }
        return nullptr;
    }  

    static std::string GetDataPath(const std::string data_file) {
        return std::string(TEST_FILE_LOCATION_DATBASE_DATA_DIR + "/" + data_file);
    }
};

class FileLocationDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        log4cxx::PropertyConfigurator::configure("log4cxx.properties");
    }
};

/// @brief Test case to test MockFileLocationDatabase::GetLocationByName() which is used by the other tests
TEST_F(FileLocationDatabaseTest, TestMockGetLocationByName)
{
    MockFileLocationDatabase test_db("mock_file.csv"); 

    Location london("London", 5);
    Location glasgow("Glasgow", 3);
    Location brighton("Brighton", 1);

    const std::vector<Location*> locations = {&london, &glasgow, &brighton};

    // Check finding each of the values, and they are the correct pointer
    EXPECT_EQ(&london, test_db.MockGetLocationByName(locations, "London"));
    EXPECT_EQ(&glasgow, test_db.MockGetLocationByName(locations, "Glasgow"));
    EXPECT_EQ(&brighton, test_db.MockGetLocationByName(locations, "Brighton"));
}

/// @brief Test case for FileLocationDatabase::GetLocationsOnDisk() For testing successfully loading the locations from disk
TEST_F(FileLocationDatabaseTest, TestLocationsOnDiskSuccess)
{
    const std::string test_data_file = MockFileLocationDatabase::GetDataPath("test_load_success.csv");
    MockFileLocationDatabase test_db(test_data_file);

    auto locations = test_db.RealGetLocationsOnDisk();
    
    EXPECT_EQ(3, locations.size());

    // Check the 3 cities are loaded and have correct info
    auto london = test_db.MockGetLocationByName(locations, "London");

    EXPECT_NE(nullptr, london);
    EXPECT_EQ("London", london->Name());
    EXPECT_EQ(5, london->Cost());

    auto glasgow = test_db.MockGetLocationByName(locations, "Glasgow");

    EXPECT_NE(nullptr, glasgow);
    EXPECT_EQ("Glasgow", glasgow->Name());
    EXPECT_EQ(3, glasgow->Cost());

    auto brighton = test_db.MockGetLocationByName(locations, "Brighton");

    EXPECT_NE(nullptr, brighton);
    EXPECT_EQ("Brighton", brighton->Name());
    EXPECT_EQ(1, brighton->Cost());    
}

/// @brief Test case for FileLocationDatabase::GetLocationsOnDisk() For testing the locations source file not existing
TEST_F(FileLocationDatabaseTest, TestLocationsOnDiskErrorFileMissing)
{
    const std::string test_data_file = MockFileLocationDatabase::GetDataPath("test_non_existant_file.csv");
    MockFileLocationDatabase test_db(test_data_file);

    EXPECT_CALL(test_db, DeleteLocations(testing::_)).Times(0);

    auto locations = test_db.RealGetLocationsOnDisk();

    EXPECT_EQ(0, locations.size());
}

/// @brief Test case for FileLocationDatabase::GetLocationsOnDisk() For testing a parsing error during loading the locations
TEST_F(FileLocationDatabaseTest, TestLocationsOnDiskErrorWhileParsing)
{
    const std::string test_data_file = MockFileLocationDatabase::GetDataPath("test_load_bad_data.csv");
    MockFileLocationDatabase test_db(test_data_file);

    EXPECT_CALL(test_db, DeleteLocations(testing::_)).Times(1).WillOnce([&test_db](std::vector<Location*>& locations){
        test_db.RealDeleteLocations(locations);
    }); 

    auto locations = test_db.RealGetLocationsOnDisk();

    EXPECT_EQ(0, locations.size());
}

/// @brief Test case for FileLocationDatabase::Load() For a successful load from disk
TEST_F(FileLocationDatabaseTest, TestLocationsLoadSuccess)
{
    MockFileLocationDatabase test_db;

    Location london("London", 5);
    Location glasgow("Glasgow", 3);
    Location brighton("Brighton", 1);

    const std::vector<Location*> locations = {&london, &glasgow, &brighton};

    EXPECT_CALL(test_db, GetLocationsOnDisk()).WillOnce([locations](){
        return locations;
    }); 

    EXPECT_CALL(test_db, DeleteLocations(testing::_)).WillOnce([](const std::vector<Location*>& locations) {
        EXPECT_EQ(0, locations.size());
    }); 

    EXPECT_CALL(test_db, AddLocation(testing::_))
    .Times(3)
    .WillOnce([&london](Location* const new_location) {
        EXPECT_EQ(&london, new_location);
    })
    .WillOnce([&glasgow](Location* const new_location) {
        EXPECT_EQ(&glasgow, new_location);
    })
    .WillOnce([&brighton](Location* const new_location) {
        EXPECT_EQ(&brighton, new_location);
    });

    bool result = test_db.Load();
    EXPECT_TRUE(result);
}

/// @brief Test case for FileLocationDatabase::Load() For a failed load from disk
TEST_F(FileLocationDatabaseTest, TestLocationsLoadFailed)
{
    MockFileLocationDatabase test_db;

    EXPECT_CALL(test_db, GetLocationsOnDisk()).WillOnce([](){
        return std::vector<Location*>();
    }); 

    EXPECT_CALL(test_db, DeleteLocations(testing::_)).Times(0);

    EXPECT_CALL(test_db, AddLocation(testing::_)).Times(0);

    bool result = test_db.Load();
    EXPECT_FALSE(result);
}