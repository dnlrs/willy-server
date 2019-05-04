#include "mac_addr.h"
#include "xml_processor.h"
#include "xml_macros.h"
#include "xml_anchors.h"
#include "xml_properties.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include <vector>
#include <iostream>
#include <sstream>

void 
cfg::xml::xml_processor::unmarshall(
    const std::string in,
    std::map<mac_addr, std::pair<double, double>>& anchors,
    std::map<std::string, std::string>& properties)
{
    std::vector<char> data(in.begin(), in.end());
    data.push_back(0);
    try {
        rapidxml::xml_document<> doc;
        doc.parse<rapidxml::parse_default>(&data.front());

        rapidxml::xml_node<>* xn_configration =
            doc.first_node(XML_ROOT);
        IF_NULL_THROW_INVALID_XML(xn_configration, "missing root element");

        xml_anchors x_anchors;
        xml_properties x_properties;
        anchors = 
            x_anchors.unmarshall(xn_configration->first_node(XML_ANCHORS));
        properties =
            x_properties.unmarshall(xn_configration->first_node(XML_PROPERTIES));
    }
    catch (rapidxml::parse_error pe) {
        std::cerr << pe.where<char>() << std::endl;
        throw conf_exception(pe.what(), CONF_BAD_SYNTAX_XML);
    }
}

void 
cfg::xml::xml_processor::marshall(
    std::string& out,
    const std::map<mac_addr, std::pair<double, double>>& anchors,
    const std::map<std::string, std::string>& properties)
{
    try {
        rapidxml::xml_document<> doc;

        rapidxml::xml_node<>* xn_configuation =
            doc.allocate_node(rapidxml::node_element, XML_ROOT);
        doc.append_node(xn_configuation);

        xml_anchors x_anchors;
        xml_properties x_properties;

        xn_configuation->append_node(x_anchors.marshall(doc, anchors));
        xn_configuation->append_node(x_properties.marshall(doc, properties));

        std::ostringstream oss;
        oss << doc;
        out = oss.str();
    }
    catch (std::bad_alloc bae) {
        throw conf_exception("could not allocate xml std::string", CONF_BAD_ALLOC);
    }
}