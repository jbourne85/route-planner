#include <gtest/gtest.h>
#include "route/FileLocationDatabase.h"

using namespace route;

TEST(AddTest, TestLoadSuccess)
{
    const std::string test_data_file = "test_data/route/test_load_success.csv";
    FileLocationDatabase test_db(test_data_file);

    bool loaded = test_db.Load();

    EXPECT_TRUE(loaded);

    // Check the 3 cities are loaded and have correct info
    auto london = test_db.GetLocation("London");

    EXPECT_NE(nullptr, london);
    EXPECT_EQ("London", london->Name());
    EXPECT_EQ(5, london->Cost());

    auto glasgow = test_db.GetLocation("Glasgow");

    EXPECT_NE(nullptr, glasgow);
    EXPECT_EQ("Glasgow", glasgow->Name());
    EXPECT_EQ(3, glasgow->Cost());

    auto brighton = test_db.GetLocation("Brighton");

    EXPECT_NE(nullptr, brighton);
    EXPECT_EQ("Brighton", brighton->Name());
    EXPECT_EQ(1, brighton->Cost());

    //Check the 3 cities exist in the list of locations
    auto locations = test_db.GetLocations();

    EXPECT_EQ(3, locations.size());
    
    auto find_london = std::find(locations.begin(), locations.end(), london);
    EXPECT_NE(locations.end(), find_london);

    auto find_glasgow = std::find(locations.begin(), locations.end(), glasgow);
    EXPECT_NE(locations.end(), find_glasgow);

    auto find_brighton = std::find(locations.begin(), locations.end(), brighton);
    EXPECT_NE(locations.end(), find_brighton);   
}

TEST(AddTest, TestLoadErrorFileMissing)
{
    const std::string test_data_file = "test_data/route/test_non_existant_file.csv";
    FileLocationDatabase test_db(test_data_file);

    bool loaded = test_db.Load();

    EXPECT_FALSE(loaded);

    auto locations = test_db.GetLocations();
    EXPECT_EQ(0, locations.size());
}