#ifndef XML_PROPERTIES_H_INCLUDED
#define XML_PROPERTIES_H_INCLUDED

#include "conf_exception.h"

#include "rapidxml/rapidxml.hpp"
#include <map>

#define XML_PROPERTIES      "properties"
#define XML_PROPERTIES_N    "n"
#define XML_PROPERTY        "property"
#define XML_PROPERTY_NAME   "name"
#define XML_PROPERTY_VALUE  "value"

namespace cfg
{
    namespace xml
    {
        // <properties n="1">
        //  <property>
        //   <name>property_name</name>
        //   <value>property_value</value>
        //  </property>
        // </propeties>
        class xml_properties
        {
        public:
            std::map<std::string, std::string>
                unmarshall(const rapidxml::xml_node<>* properties_node);

            rapidxml::xml_node<>*
                marshall(
                    rapidxml::xml_document<>& doc,
                    std::map<std::string, std::string> properties);
        };
    }
}
#endif // !XML_PROPERTIES_H_INCLUDED
