#include "configuration.h"
#include "xml_processor.h"
#include "conf_exception.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "utils.h"
#include <mutex>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

void
cfg::configuration::load_configuration(
    const std::string& file_name)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);

    try {
        rapidxml::file<> xml_file(file_name.c_str());

        std::map<mac_addr, std::pair<double, double>> local_anchors;
        std::map<std::string, std::string> local_properties;

        xml::xml_processor unmarshaller;
        unmarshaller.unmarshall(
            xml_file.data(), 
            local_anchors, 
            local_properties);

        // save actual configuration
        this->anchors = local_anchors;
        this->properties.clear(); // old properties are deleted
        for (auto property : local_properties)
            this->properties.insert_or_assign(property.first, property.second);
    }
    catch (std::runtime_error re) {
        throw conf_exception(re.what(), CONF_FILE_NOT_FOUND);
    }
}

void
cfg::configuration::save_configuration(
    const std::string& file_name)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);

    std::string rval;
    xml::xml_processor marshaller;
    marshaller.marshall(
        rval, 
        this->anchors, 
        this->properties);

    std::ofstream file;
    file.open(file_name, std::ios_base::trunc | std::ios_base::out);

    if (file.is_open() == false) {
        file.close();
        throw conf_exception("could not open output file", CONF_FILE_NOT_CREATED);
    }

    // write configuration on file
    file << rval << std::endl;

    if (file.good() == false) {
        file.close();
        throw conf_exception("could not marshall data", CONF_MARSHALL_ERROR);
    }

    file.close();
}


bool
cfg::configuration::get_property(
    const std::string& property_name, 
    std::string& rval)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);

    if (property_name.empty())
        return false;

    if (properties.find(property_name) == properties.end())
        return false;

    rval = properties.at(property_name);
    return true;
}

bool
cfg::configuration::set_property(
    const std::string& property_name, 
    const std::string& property_value)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);

    if (property_name.empty() || property_value.empty())
        return false;

    properties.insert_or_assign(property_name, property_value);
    return true;
}

bool
cfg::configuration::del_property(const std::string& name)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);

    if (properties.find(name) == properties.end())
        return false;

    properties.erase(name);
    return true;
}

int
cfg::configuration::get_anchors_number()
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);
    return (int) anchors.size();
}

bool
cfg::configuration::anchor_is_known(const mac_addr mac)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex> 
        guard(configuration_access);
    
    return ((anchors.find(mac) != anchors.end()) ? true : false);
}

bool
cfg::configuration::get_anchor_position(
    const mac_addr mac,
    std::pair<double, double>& rval)
{
    // lock configuration
    std::lock_guard<std::recursive_mutex>
        guard(configuration_access);

    if (!anchor_is_known(mac))
        return false;

    rval = anchors.at(mac);
    return true;
}