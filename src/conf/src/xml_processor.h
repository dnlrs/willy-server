#ifndef XML_PROCESSOR_H_INCLUDED
#define XML_PROCESSOR_H_INCLUDED

#include <string>
#include <map>

#define XML_ROOT "configuration"

namespace cfg
{
    namespace xml
    {
        class xml_processor 
        {
        public:
            void unmarshall(
                const std::string in,
                std::map<uint64_t, std::pair<double, double>>& anchors,
                std::map<std::string, std::string>& properties);

            void marshall(
                std::string& out,
                const std::map<uint64_t, std::pair<double, double>>& anchors,
                const std::map<std::string, std::string>& properties);
        };
    }
}


#endif // !XML_PROCESSOR_H_INCLUDED
