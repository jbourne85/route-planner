#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "route/FileRouteDatabase.h"

using namespace route;

const std::string TEST_FILE_ROUTE_DATBASE_DATA_DIR("test_data/route/test_file_route_database");

/// @brief this is the Mock File Route Database class, its used to mock and control the routes read from disk
class MockFileRouteDatabase : public FileRouteDatabase {
public:
    MockFileRouteDatabase() : FileRouteDatabase("test_data.csv") {}
    MockFileRouteDatabase(const std::string route_file) : FileRouteDatabase(route_file) {}

    // MOCK_METHOD(std::vector<const Location* const>, GetLocationsOnDisk, (), (override));
    MOCK_METHOD((std::unordered_map<std::string, std::vector<std::string>>), GetRoutesOnDisk, (), (const, override));

    /// @brief Adapter method to call FileRouteDatabase::GetRoutesOnDisk from the mock
    std::unordered_map<std::string, std::vector<std::string>> RealGetRoutesOnDisk() {
        return FileRouteDatabase::GetRoutesOnDisk();
    }

    /// @brief Utility method to return a Route by name, useful for simplifying the test code
    /// @param routes the map of starting locations to end locations
    /// @param name The name of the starting location to find
    /// @return The routes on success, or an emppyt list on failure
    std::vector<std::string> MockGetRoutesByName(std::unordered_map<std::string, std::vector<std::string>>& routes, std::string name) {
        std::vector<std::string> results;
        std::for_each (routes.begin(), routes.end(), [this, &results, name](std::pair<std::string, std::vector<std::string>> route) mutable {
            if (route.first == name) {
                results = route.second;
            }
            
        });
        return results;
    }  

    static std::string GetDataPath(const std::string data_file) {
        return std::string(TEST_FILE_ROUTE_DATBASE_DATA_DIR + "/" + data_file);
    }
};

/// @brief Test case to test MockFileLocationDatabase::GetLocationByName() which is used by the other tests
TEST(AddTest, TestMockGetRoutesByName)
{
    MockFileRouteDatabase test_db; 

    std::unordered_map<std::string, std::vector<std::string>> routes;

    std::vector<std::string> london_routes = {"Brighton", "Bath", "Oxford", "Leicester", "Cambridge"};
    routes.insert(std::make_pair("London", london_routes));

    std::vector<std::string> manchester_routes = {"Birmingham", "Liverpool", "Sheffield"};
    routes.insert(std::make_pair("Manchester", manchester_routes));

    std::vector<std::string> johnogroates_routes = {"Glasgow", "Endinburgh"};
    routes.insert(std::make_pair("John O' Groats", johnogroates_routes));

    london_routes = test_db.MockGetRoutesByName(routes, "London");
    EXPECT_EQ(5, london_routes.size());
    EXPECT_EQ("Brighton", london_routes[0]);
    EXPECT_EQ("Bath",   london_routes[1]);
    EXPECT_EQ("Oxford", london_routes[2]);
    EXPECT_EQ("Leicester", london_routes[3]);
    EXPECT_EQ("Cambridge", london_routes[4]);

    manchester_routes = test_db.MockGetRoutesByName(routes, "Manchester");
    EXPECT_EQ(3, manchester_routes.size());
    EXPECT_EQ("Birmingham", manchester_routes[0]);
    EXPECT_EQ("Liverpool", manchester_routes[1]);
    EXPECT_EQ("Sheffield", manchester_routes[2]);

    johnogroates_routes = test_db.MockGetRoutesByName(routes, "John O' Groats");
    EXPECT_EQ(2, johnogroates_routes.size());
    EXPECT_EQ("Glasgow", johnogroates_routes[0]);
    EXPECT_EQ("Endinburgh", johnogroates_routes[1]);
}

/// @brief Test case for MockFileRouteDatabase::GetRoutesOnDisk() For testing successfully loading the routes from disk
TEST(AddTest, TestRoutesOnDiskSuccess)
{
    const std::string test_data_file = MockFileRouteDatabase::GetDataPath("test_load_success.csv");
    MockFileRouteDatabase test_db(test_data_file);

    auto routes = test_db.RealGetRoutesOnDisk();
    
    EXPECT_EQ(3, routes.size());

    // Check the 3 cities and their routes are correct
    auto london_routes = test_db.MockGetRoutesByName(routes, "London");
    EXPECT_EQ(5, london_routes.size());
    EXPECT_EQ("Brighton", london_routes[0]);
    EXPECT_EQ("Bath",   london_routes[1]);
    EXPECT_EQ("Oxford", london_routes[2]);
    EXPECT_EQ("Leicester", london_routes[3]);
    EXPECT_EQ("Cambridge", london_routes[4]);

    auto manchester_routes = test_db.MockGetRoutesByName(routes, "Manchester");
    EXPECT_EQ(3, manchester_routes.size());
    EXPECT_EQ("Birmingham", manchester_routes[0]);
    EXPECT_EQ("Liverpool", manchester_routes[1]);
    EXPECT_EQ("Sheffield", manchester_routes[2]);

    auto johnogroates_routes = test_db.MockGetRoutesByName(routes, "John O' Groats");
    EXPECT_EQ(2, johnogroates_routes.size());
    EXPECT_EQ("Glasgow", johnogroates_routes[0]);
    EXPECT_EQ("Endinburgh", johnogroates_routes[1]);
}

/// @brief Test case for FileRouteDatabase::GetRoutesOnDisk() For testing the routes source file not existing
TEST(AddTest, TestRoutesOnDiskErrorFileMissing)
{
    const std::string test_data_file = MockFileRouteDatabase::GetDataPath("test_non_existant_file.csv");
    MockFileRouteDatabase test_db(test_data_file);

    auto routes = test_db.RealGetRoutesOnDisk();

    EXPECT_EQ(0, routes.size());
}

/// @brief Test case for FileRouteDatabase::Load() For a successful load from disk
TEST(AddTest, TestRoutesLoadSuccess)
{
    MockFileRouteDatabase test_db;

    std::unordered_map<std::string, std::vector<std::string>> routes;

    std::vector<std::string> london_routes = {"Brighton", "Bath", "Oxford", "Leicester", "Cambridge"};
    routes.insert(std::make_pair("London", london_routes));

    std::vector<std::string> manchester_routes = {"Birmingham", "Liverpool", "Sheffield"};
    routes.insert(std::make_pair("Manchester", manchester_routes));

    std::vector<std::string> johnogroates_routes = {"Glasgow", "Endinburgh"};
    routes.insert(std::make_pair("John O' Groats", johnogroates_routes));

    EXPECT_CALL(test_db, GetRoutesOnDisk()).WillOnce([routes](){
        return routes;
    }); 

    bool result = test_db.Load();
    EXPECT_TRUE(result);

    // Check the 3 cities and their routes are correct
    london_routes = test_db.GetRoutes("London");
    EXPECT_EQ(5, london_routes.size());
    EXPECT_EQ("Brighton", london_routes[0]);
    EXPECT_EQ("Bath",   london_routes[1]);
    EXPECT_EQ("Oxford", london_routes[2]);
    EXPECT_EQ("Leicester", london_routes[3]);
    EXPECT_EQ("Cambridge", london_routes[4]);

    manchester_routes = test_db.GetRoutes("Manchester");
    EXPECT_EQ(3, manchester_routes.size());
    EXPECT_EQ("Birmingham", manchester_routes[0]);
    EXPECT_EQ("Liverpool", manchester_routes[1]);
    EXPECT_EQ("Sheffield", manchester_routes[2]);

    johnogroates_routes = test_db.GetRoutes("John O' Groats");
    EXPECT_EQ(2, johnogroates_routes.size());
    EXPECT_EQ("Glasgow", johnogroates_routes[0]);
    EXPECT_EQ("Endinburgh", johnogroates_routes[1]);
}

/// @brief Test case for FileRouteDatabase::Load() For a failed load from disk
TEST(AddTest, TestRoutesLoadFailed)
{
    MockFileRouteDatabase test_db;

    EXPECT_CALL(test_db, GetRoutesOnDisk()).WillOnce([](){
        return std::unordered_map<std::string, std::vector<std::string>>();
    }); 

    bool result = test_db.Load();
    EXPECT_FALSE(result);
}