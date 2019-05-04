#ifndef XML_ANCHOR_H_INCLUDED
#define XML_ANCHOR_H_INCLUDED

#include "mac_addr.h"
#include "rapidxml/rapidxml.hpp"
#include <map>

#define XML_ANCHORS           "anchors"
#define XML_ANCHORS_N         "n"
#define XML_ANCHOR            "anchor"
#define XML_ANCHOR_MAC        "mac"
#define XML_ANCHOR_POSITION   "position"
#define XML_ANCHOR_POSITION_X "x"
#define XML_ANCHOR_POSITION_Y "y"

namespace cfg
{
    namespace xml
    {
        // <anchors n="1">
        //  <anchor>
        //   <mac>00:00:00:00:00:00</mac>
        //   <position>
        //     <x>0.000000</x>
        //     <y>0.000000</y>
        //   </position>
        //  <anchor>
        // </anchors>
        class xml_anchors
        {
        public:
            std::map<mac_addr, std::pair<double, double>> 
                unmarshall(const rapidxml::xml_node<>* anchors_node);

            rapidxml::xml_node<>* 
                marshall(
                    rapidxml::xml_document<>& doc,
                    std::map<mac_addr, std::pair<double, double>> anchors);
        };
    }
}

#endif // !XML_ANCHOR_H_INCLUDED
