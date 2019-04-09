#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED

#include "conf_exception.h"
#include <string>
#include <map>
#include <mutex>
#include <utility>

#define DEFAULT_CONFIGURATION_FILE  "willy_properties.xml"
#define net_server_port             "net_server_port"
#define net_server_port_value       "27015"

namespace cfg {

    class configuration
    {
    public:

        configuration() : 
            configuration_file(DEFAULT_CONFIGURATION_FILE) 
        {
            properties[net_server_port] = net_server_port_value;
        }

        // load/save configuration from/to default file
        void load_configuration() { load_configuration(configuration_file); }
        void save_configuration() { save_configuration(configuration_file); }
        
        // import configuration from specified file
        void import_configuration(
            const std::string& file_name, 
            bool make_default)
        {
            std::lock_guard<std::recursive_mutex> lock(configuration_access);
            load_configuration(file_name);
            save_configuration();
            if ( make_default )
                configuration_file = file_name;
        }
        
        // export configuration to specified file
        void export_configuration(
            const std::string& file_name, 
            bool make_default) 
        { 
            save_configuration(file_name); 
            if (make_default)
                configuration_file = file_name;
        }

        // Adds or replaces a property if its name and value are not empty strings
        bool set_property(
            const std::string& property_name, 
            const std::string& property_value);
        
        bool get_property(const std::string& property_name, std::string& rval);
        bool del_property(const std::string& name);
        
        // properties regarding anchors
        bool anchor_is_known(const uint64_t mac);
        bool get_anchor_position(
            const uint64_t mac, 
            std::pair<double,double>& rval);
        size_t get_anchors_number();

    private:
        // load/save configuration from/to specified file
        void load_configuration(const std::string& file_name);
        void save_configuration(const std::string& file_name);

    private:
        // the configuration file to which is mapped this instance
        std::string configuration_file;

        std::recursive_mutex configuration_access;
        
        std::map<uint64_t, std::pair<double, double>> anchors;
        std::map<std::string, std::string>  properties;

    };
}
#endif // CONFIGURATION_H_INCLUDED