#include "route/Location.h"

namespace route {
Location::Location(const std::string name, unsigned int cost) :
m_name(name),
m_cost(cost)
{

}

const std::string Location::Name() const {
    return m_name;
}

const unsigned int Location::Cost() const {
    return m_cost;
}

std::vector<const Location* const> Location::Routes() const {
    return m_routes;
}
}

