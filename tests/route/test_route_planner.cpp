#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include "route/ILocationDatabase.h"
#include "route/IRouteDatabase.h"
#include "route/RoutePlanner.h"

using namespace route;

/// @brief this is the Mock Route Planner class, its used to mock and control the route planner
class MockRoutePlanner : public RoutePlanner {
public:
    MockRoutePlanner(std::shared_ptr<ILocationDatabase> location_db, std::shared_ptr<IRouteDatabase> route_db) : RoutePlanner(location_db, route_db) {}
    
    MOCK_METHOD(const std::vector<Location*>, SetupRoutes, (), (override, const));

    /// @brief Adapter method to call RoutePlanner::getRoutes from the mock
    const std::vector<Location*> RealSetupRoutes() {
        return RoutePlanner::SetupRoutes();
    }
};

/// @brief this is the Mock Location Database class, its used to mock and control the locations during testing
class MockLocationDatabase : public ILocationDatabase {
public:
    MOCK_METHOD(bool, Load,(), (override));
    MOCK_METHOD(const std::vector<Location*>, GetLocations,(), (override, const));
    MOCK_METHOD(Location* const, GetLocation,(std::string location_name), (override, const)); 
};

/// @brief this is the Mock Location Database class, its used to mock and control the locations during testing
class MockRouteDatabase : public IRouteDatabase {
public: 
    MOCK_METHOD(bool, Load,(), (override));
    MOCK_METHOD(std::vector<std::string>, GetRoutes, (const std::string start_location_name), (override, const)); 
};

class RoutePlannerTest : public ::testing::Test {
protected:
    void SetUp() override {
        log4cxx::PropertyConfigurator::configure("log4cxx.properties");

        mock_route_db = std::make_shared<MockRouteDatabase>();
        mock_location_db = std::make_shared<MockLocationDatabase>();
        route_planner = std::make_unique<MockRoutePlanner>(mock_location_db, mock_route_db);
    }

    void TearDown() override {
    }

    std::unique_ptr<MockRoutePlanner> route_planner;
    std::shared_ptr<MockRouteDatabase> mock_route_db;
    std::shared_ptr<MockLocationDatabase> mock_location_db;
};

/// @brief Test case for RoutePlanner::GetRoutes() in the success case for setting up the following routes:
/// Glasgow <--> London <--> Brighton
TEST_F(RoutePlannerTest, GetRoutesSuccess)
{   
    //setup the Load calls
    EXPECT_CALL(*mock_location_db, Load())
    .WillOnce([](){
        return true;
    });

    EXPECT_CALL(*mock_route_db, Load())
    .WillOnce([](){
        return true;
    });

    //Setup the locations
    Location london("London", 5);
    Location glasgow("Glasgow", 3);
    Location brighton("Brighton", 1);

    const std::vector<Location*> locations = {&london, &glasgow, &brighton};

    EXPECT_CALL(*mock_location_db, GetLocations()).WillOnce([locations]() {
        return locations;
    }); 

    //Setup the routes
    std::unordered_map<std::string, std::vector<std::string>> routes;

    std::vector<std::string> london_routes = {"Glasgow", "Brighton"};
    routes.insert(std::make_pair("London", london_routes));

    std::vector<std::string> glasgow_routes = {"London"};
    routes.insert(std::make_pair("Glasgow", glasgow_routes));

    std::vector<std::string> brighton_routes = {"London"};
    routes.insert(std::make_pair("Brighton", brighton_routes));

    EXPECT_CALL(*mock_route_db, GetRoutes(testing::_)).WillRepeatedly([routes](const std::string start_location) {
        return (routes.find(start_location))->second;
    }); 

    //Setup the individual location fetch
    EXPECT_CALL(*mock_location_db, GetLocation(testing::_)).WillRepeatedly([locations](const std::string location_name) {
        Location* location_match = nullptr;
        std::for_each(locations.begin(), locations.end(), [&location_match, location_name](Location* const location) -> void {
            if (location->Name() == location_name) {
                location_match = location;
            }
        });
        return location_match;
    }); 

    auto calculated_routes = route_planner->RealSetupRoutes();

    EXPECT_EQ(calculated_routes.size(), 3);

    EXPECT_EQ(london.Destinations()["Glasgow"], &glasgow);
    EXPECT_EQ(london.Destinations()["Brighton"], &brighton);
    EXPECT_EQ(glasgow.Destinations()["London"], &london);
    EXPECT_EQ(brighton.Destinations()["London"], &london);
}

/// @brief Test case for RoutePlanner::GetRoutes() in the success case for when the location database changes
TEST_F(RoutePlannerTest, TestGetRoutesRereshOnLocationDbChange)
{   
    //setup the Load calls
    EXPECT_CALL(*mock_location_db, Load())
    .WillRepeatedly([](){
        return true;
    });

    EXPECT_CALL(*mock_route_db, Load())
    .WillRepeatedly([](){
        return false;
    });

    Location london("London", 5);
    const std::vector<Location*> locations = {&london};

    //Just assert that the locations are fetched
    EXPECT_CALL(*mock_location_db, GetLocations())
    .Times(1)
    .WillOnce([locations]() {
        return locations;
    }); 

    //Only interested in checking the route calculation gets called
    EXPECT_CALL(*mock_route_db, GetRoutes(testing::_))
    .Times(1)
    .WillOnce([](const std::string start_location) {
        return std::vector<std::string>();
    }); 

    auto calculated_routes = route_planner->RealSetupRoutes();
    EXPECT_EQ(calculated_routes.size(), 1);
}

/// @brief Test case for RoutePlanner::GetRoutes() in the success case for when the route database changes
TEST_F(RoutePlannerTest, TestGetRoutesRereshOnRouteDbChange)
{   
    //setup the Load calls
    EXPECT_CALL(*mock_location_db, Load())
    .WillRepeatedly([](){
        return false;
    });

    EXPECT_CALL(*mock_route_db, Load())
    .WillRepeatedly([](){
        return true;
    });

    Location london("London", 5);
    const std::vector<Location*> locations = {&london};

    //Just assert that the locations are fetched
    EXPECT_CALL(*mock_location_db, GetLocations())
    .Times(1)
    .WillOnce([locations]() {
        return locations;
    }); 

    //Only interested in checking the route calculation gets called
    EXPECT_CALL(*mock_route_db, GetRoutes(testing::_))
    .Times(1)
    .WillOnce([](const std::string start_location) {
        return std::vector<std::string>();
    }); 

    auto calculated_routes = route_planner->RealSetupRoutes();
    EXPECT_EQ(calculated_routes.size(), 1);
}

/// @brief Test case for RoutePlanner::GetRoutes() in the success case for when beither db has changed
TEST_F(RoutePlannerTest, TestGetRoutesNoDbChange)
{   
    //setup the Load calls
    EXPECT_CALL(*mock_location_db, Load())
    .WillRepeatedly([](){
        return false;
    });

    EXPECT_CALL(*mock_route_db, Load())
    .WillRepeatedly([](){
        return false;
    });

    Location london("London", 5);
    const std::vector<Location*> locations = {&london};

    //Just assert that the locations are fetched
    EXPECT_CALL(*mock_location_db, GetLocations())
    .Times(1)
    .WillOnce([locations]() {
        return locations;
    }); 

    //Only interested in checking the route has not been recalculated
    EXPECT_CALL(*mock_route_db, GetRoutes(testing::_))
    .Times(0); 

    auto calculated_routes = route_planner->RealSetupRoutes();
    EXPECT_EQ(calculated_routes.size(), 1);
}

/// @brief Test case for RoutePlanner::GetLocationNames() in the success case for when there are locations avaliable
TEST_F(RoutePlannerTest, TestGetLocationNames)
{   
    //Setup the locations
    Location london("London", 5);
    Location glasgow("Glasgow", 3);
    Location brighton("Brighton", 1);

    const std::vector<Location*> locations = {&london, &glasgow, &brighton};

    EXPECT_CALL(*route_planner, SetupRoutes())
    .Times(1)
    .WillOnce([locations]() {
        return locations;
    }); 

    auto location_names = route_planner->GetLocationNames();

    EXPECT_EQ(location_names.size(), 3);
    EXPECT_EQ(location_names[0], "London");
    EXPECT_EQ(location_names[1], "Glasgow");
    EXPECT_EQ(location_names[2], "Brighton");
}