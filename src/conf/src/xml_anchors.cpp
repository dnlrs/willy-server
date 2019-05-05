#include "xml_anchors.h"
#include "xml_macros.h"
#include "conf_exception.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "utils.h"
#include <map>
#include <utility>
#include <string>

std::map<mac_addr, std::pair<double, double>> 
cfg::xml::xml_anchors::unmarshall(
    const rapidxml::xml_node<>* anchors_node)
{
    IF_NULL_THROW_INVALID_XML(anchors_node, "missing anchors element");

    // read anchors number
    rapidxml::xml_attribute<>* xa_anchors_number =
        anchors_node->first_attribute(XML_ANCHORS_N);

    IF_NULL_THROW_INVALID_XML(
        xa_anchors_number, 
        "anchors number attribute missing");

    IF_NULL_THROW_INVALID_XML(
        str_is_valid_int(xa_anchors_number->value()),
        "anchors number is not a valid integer");

    int anchors_number = std::stoi(xa_anchors_number->value());
    if (anchors_number <= 0)
        throw conf_exception("invalid number of anchors", CONF_NO_ANCHORS);

    // for each anchor 
    // - read its mac
    // - read tts position
    std::map<mac_addr, std::pair<double, double>> anchors;
    for (rapidxml::xml_node<>* xn_anchor = anchors_node->first_node(XML_ANCHOR);
        xn_anchor;
        xn_anchor = xn_anchor->next_sibling())
    {
        // read ancor's MAC
        rapidxml::xml_node<>* mac_node = xn_anchor->first_node(XML_ANCHOR_MAC);
        IF_NULL_THROW_INVALID_XML(mac_node, "'mac' element missing");

        mac_addr mac(mac_node->value());
        IF_NULL_THROW_INVALID_XML(mac.is_valid(), "'mac' value missing");

        // check for doubles (if 2 anchors have same MAC)
        if (anchors.find(mac) != anchors.end())
            throw conf_exception(
                "found two anchors with same mac", 
                CONF_INVALID_XML);

        // read anchor's position
        rapidxml::xml_node<>* pos_node = 
            xn_anchor->first_node(XML_ANCHOR_POSITION);
        IF_NULL_THROW_INVALID_XML(pos_node, "'position' element missing");

        // read x coordinate
        rapidxml::xml_node<>* coordinate_node = 
            pos_node->first_node(XML_ANCHOR_POSITION_X);

        IF_NULL_THROW_INVALID_XML(coordinate_node, "'x' element missing");

        IF_NULL_THROW_INVALID_XML(
            str_is_valid_double(coordinate_node->value()),
            "'x' value is invalid double");

        double x = std::stod(coordinate_node->value());

        // read y coordinate
        coordinate_node = pos_node->first_node(XML_ANCHOR_POSITION_Y);
        IF_NULL_THROW_INVALID_XML(coordinate_node, "'y' element missing");
        IF_NULL_THROW_INVALID_XML(
            str_is_valid_double(coordinate_node->value()),
            "'y' value is invalid double");

        double y = std::stod(coordinate_node->value());

        // save locally anchor
        anchors[mac] = std::make_pair(x, y);
    }

    if (anchors.size() != anchors_number)
        throw conf_exception("inconsistent number of anchors", CONF_INVALID_XML);

    return anchors;
}

rapidxml::xml_node<>*
cfg::xml::xml_anchors::marshall(
    rapidxml::xml_document<>& doc,
    std::map<mac_addr, std::pair<double, double>> anchors)
{
    rapidxml::xml_node<>* anchors_node;

    try {
        // allocate anchors' node
         anchors_node = doc.allocate_node(rapidxml::node_element, XML_ANCHORS);

        // add number of anchors attribute
        char* anchors_n_value =
            doc.allocate_string(std::to_string(anchors.size()).c_str());
        rapidxml::xml_attribute<>* anchors_n_attr =
            doc.allocate_attribute(XML_ANCHORS_N, anchors_n_value);
        anchors_node->append_attribute(anchors_n_attr);

        // build element for each anchor
        for (auto entry : anchors)
        {
            std::pair<double, double> position = entry.second;
            char* mac_value =
                doc.allocate_string((entry.first).str().c_str());
            char* x_value =
                doc.allocate_string(std::to_string(position.first).c_str());
            char* y_value =
                doc.allocate_string(std::to_string(position.second).c_str());

            // add anchor element
            rapidxml::xml_node<>* anchor_node =
                doc.allocate_node(rapidxml::node_element, XML_ANCHOR);
            anchors_node->append_node(anchor_node);

            // add MAC element
            rapidxml::xml_node<>* mac_node =
                doc.allocate_node(rapidxml::node_element, XML_ANCHOR_MAC, mac_value);
            anchor_node->append_node(mac_node);

            // add position element
            rapidxml::xml_node<>* position_node =
                doc.allocate_node(rapidxml::node_element, XML_ANCHOR_POSITION);
            anchor_node->append_node(position_node);

            // add x-coordinate element
            rapidxml::xml_node<>* x_node =
                doc.allocate_node(rapidxml::node_element, XML_ANCHOR_POSITION_X, x_value);
            position_node->append_node(x_node);

            // add y-coordinate element
            rapidxml::xml_node<>* y_node =
                doc.allocate_node(rapidxml::node_element, XML_ANCHOR_POSITION_Y, y_value);
            position_node->append_node(y_node);
        }
    }
    catch (std::bad_alloc bae) {
        throw conf_exception("could not allocate xml std::string", CONF_BAD_ALLOC);
    }

    return anchors_node;
}