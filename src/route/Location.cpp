#include <utility>
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

Location::ValidDestinationsType Location::Destinations() const {
    return m_destinations;
}

void Location::AddDestination(const Location *const destination) {
    m_destinations.insert(std::make_pair(destination->Name(), destination));
}

bool Location::DestinationIsValid(const Location *const destination) const {
    auto location = m_destinations.find(destination->Name());
    return location != m_destinations.end();
}

}
