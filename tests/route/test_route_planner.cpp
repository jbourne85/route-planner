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

/// @brief Test case for RoutePlanner::GetRouteCost() in the success case for when the start, end locations is valid
TEST_F(RoutePlannerTest, TestCalculateRouteCost)
{   
    //Setup the locations (use the locations from the assignment breif)
    Location location_1("London", 5);
    Location location_2("Birmingham", 5);
    Location location_3("Manchester", 5);
    Location location_4("Liverpool", 5);
    Location location_5("Glasgow", 3);
    Location location_6("Leeds", 3);
    Location location_7("Edinburgh", 3);
    Location location_8("Peterborough", 3);
    Location location_9("Newcastle", 3);
    Location location_10("Bath", 3);
    Location location_11("Brighton", 1);
    Location location_12("Leicester", 1);
    Location location_13("Oxford", 1);
    Location location_14("Cambridge", 1);
    Location location_15("Sheffield", 1);
    Location location_16("John O’Groats", 1);

    const std::vector<Location*> locations = {
        &location_1, 
        &location_2, 
        &location_3, 
        &location_4, 
        &location_5, 
        &location_6, 
        &location_7, 
        &location_8, 
        &location_9, 
        &location_10, 
        &location_11, 
        &location_12, 
        &location_13, 
        &location_14, 
        &location_15, 
        &location_16, 
    };

    EXPECT_CALL(*mock_location_db, GetLocations()).WillRepeatedly([locations]() {
        return locations;
    }); 

    EXPECT_CALL(*mock_location_db, GetLocation(testing::_)).WillRepeatedly([locations](const std::string location_name) {
        Location* location_match = nullptr;
        std::for_each(locations.begin(), locations.end(), [&location_match, location_name](Location* const location) -> void {
            if (location->Name() == location_name) {
                location_match = location;
            }
        });
        return location_match;
    }); 


    std::unordered_map<std::string, std::vector<std::string>> routes;

    routes.insert(std::make_pair("London", std::vector<std::string>({"Leicester", "Cambridge", "Brighton", "Bath", "Oxford"})));
    routes.insert(std::make_pair("Birmingham", std::vector<std::string>({"Liverpool", "Manchester", "Leicester", "Oxford"})));
    routes.insert(std::make_pair("Manchester", std::vector<std::string>({"Sheffield", "Birmingham", "Liverpool"})));
    routes.insert(std::make_pair("Liverpool", std::vector<std::string>({"Glasgow", "Edinburgh", "Manchester", "Birmingham"})));
    routes.insert(std::make_pair("Glasgow", std::vector<std::string>({"John O’Groats", "Edinburgh", "Liverpool"})));
    routes.insert(std::make_pair("Leeds", std::vector<std::string>({"Newcastle", "Sheffield"})));
    routes.insert(std::make_pair("Edinburgh", std::vector<std::string>({"Glasgow", "John O’Groats", "Newcastle", "Liverpool"})));
    routes.insert(std::make_pair("Peterborough", std::vector<std::string>({"Sheffield", "Cambridge"})));
    routes.insert(std::make_pair("Newcastle", std::vector<std::string>({"Edinburgh", "Leeds"})));
    routes.insert(std::make_pair("Bath", std::vector<std::string>({"Oxford", "London", "Brighton"})));
    routes.insert(std::make_pair("Brighton",  std::vector<std::string>({"London", "Bath"})));
    routes.insert(std::make_pair("Leicester", std::vector<std::string>({"Sheffield", "Cambridge", "London", "Birmingham"})));
    routes.insert(std::make_pair("Oxford", std::vector<std::string>({"Birmingham", "London", "Bath"})));
    routes.insert(std::make_pair("Cambridge", std::vector<std::string>({"Leicester", "Peterborough", "London"})));
    routes.insert(std::make_pair("Sheffield", std::vector<std::string>({"Manchester", "Leeds", "Peterborough", "Leicester"})));
    routes.insert(std::make_pair("John O’Groats", std::vector<std::string>({"Edinburgh", "Glasgow"})));

    EXPECT_CALL(*mock_route_db, GetRoutes(testing::_)).WillRepeatedly([routes](const std::string start_location) {
        return (routes.find(start_location))->second;
    }); 

    EXPECT_CALL(*mock_location_db, Load())
    .WillRepeatedly([](){
        return false;
    });

    EXPECT_CALL(*mock_route_db, Load())
    .WillRepeatedly([](){
        return true;
    });

    route_planner->RealSetupRoutes();

    EXPECT_EQ(route_planner->GetRouteCost("John O’Groats", "Liverpool"), 9);
    EXPECT_EQ(route_planner->GetRouteCost("Brighton", "Oxford"), 5);
    EXPECT_EQ(route_planner->GetRouteCost("Leeds", "Cambridge"), 6);
    EXPECT_EQ(route_planner->GetRouteCost("Manchester", "Brighton"), 13);
}