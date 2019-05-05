#include "xml_properties.h"
#include "xml_macros.h"
#include "conf_exception.h"
#include "rapidxml/rapidxml_utils.hpp"
#include "utils.h"
#include <map>
#include <string>

std::map<std::string, std::string>
cfg::xml::xml_properties::unmarshall(
    const rapidxml::xml_node<>* properties_node)
{
    IF_NULL_THROW_INVALID_XML(properties_node, "'properties' element missing");

    // read properties' number
    rapidxml::xml_attribute<>* xa_properties_number = 
        properties_node->first_attribute(XML_PROPERTIES_N);

    IF_NULL_THROW_INVALID_XML(
        xa_properties_number, 
        "properties number attribute missing");

    IF_NULL_THROW_INVALID_XML(
        str_is_valid_int(xa_properties_number->value()), 
        "properties number is not a valid integer");

    int properties_number = std::stoi(xa_properties_number->value());

    // for each property
    // - read its name
    // - read its value
    std::map<std::string, std::string> properties;
    if (properties_number > 0) {
        for (rapidxml::xml_node<>* xn_property = properties_node->first_node(XML_PROPERTY);
            xn_property;
            xn_property = xn_property->next_sibling())
        {
            // read property's name
            rapidxml::xml_node<>* xn_property_name = 
                xn_property->first_node(XML_PROPERTY_NAME);

            IF_NULL_THROW_INVALID_XML(
                xn_property_name, 
                "property 'name' element missing");

            IF_NULL_THROW_INVALID_XML(
                xn_property_name->value(), 
                "property 'name' value missing");
            
            std::string name = xn_property_name->value();

            // check if same property was defined twice
            if (properties.find(name) != properties.end())
                throw conf_exception("same property defined twice", CONF_INVALID_XML);

            // read property's value
            rapidxml::xml_node<>* xn_property_value = 
                xn_property->first_node("value");

            IF_NULL_THROW_INVALID_XML(
                xn_property_value, 
                "property 'value' element missing");

            IF_NULL_THROW_INVALID_XML(
                xn_property_value->value(), 
                "property 'value' value missing");
            
            std::string value = xn_property_value->value();

            // save locally property
            properties[name] = value;
        }

        if (properties.size() != properties_number)
            throw conf_exception("inconsistent number of properties", CONF_INVALID_XML);
    }

    return properties;
}

rapidxml::xml_node<>*
cfg::xml::xml_properties::marshall(
    rapidxml::xml_document<>& doc,
    std::map<std::string, std::string> properties)
{
    rapidxml::xml_node<>* properties_node;
    try {
        properties_node = doc.allocate_node(rapidxml::node_element, XML_PROPERTIES);
    
        // add number of properties attribute
        char* properties_n_value = 
            doc.allocate_string(std::to_string(properties.size()).c_str());
        rapidxml::xml_attribute<>* properties_n_attr = 
            doc.allocate_attribute(XML_PROPERTIES_N, properties_n_value);
        properties_node->append_attribute(properties_n_attr);

        // build element fr each property
        for (auto entry : properties)
        {
            char* name  = doc.allocate_string(entry.first.c_str());
            char* value = doc.allocate_string(entry.second.c_str());

            // add property element
            rapidxml::xml_node<>* property_node = 
                doc.allocate_node(rapidxml::node_element, XML_PROPERTY);
            properties_node->append_node(property_node);

            // add property name node
            rapidxml::xml_node<>* name_node = 
                doc.allocate_node(rapidxml::node_element, XML_PROPERTY_NAME, name);
            property_node->append_node(name_node);

            // add property value node
            rapidxml::xml_node<>* value_node = 
                doc.allocate_node(rapidxml::node_element, XML_PROPERTY_VALUE, value);
            property_node->append_node(value_node);
        }
    }
    catch (std::bad_alloc bae) {
        throw conf_exception("could not allocate xml std::string", CONF_BAD_ALLOC);
    }

    return properties_node;
}