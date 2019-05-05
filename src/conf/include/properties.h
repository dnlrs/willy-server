#ifndef PROPERTIES_H_INCLUDED
#define PROPERTIES_H_INCLUDED

#include <string>
#include <map>

// server properties
#define net_server_port "net_server_port"


//core properties
#define core_min_threads_nr "core_min_threads_nr"
#define core_max_threads_nr "core_max_threads_nr"
#define core_ini_threads_nr "core_ini_threads_nr"

const std::map<std::string, std::string> default_properties = 
{
    
/* server listening port */
{net_server_port, "27015"},
   
/* minimum number of application threads deployed */
{core_min_threads_nr, "2"},

/*maximum number of application threads deployed */
{core_max_threads_nr, "16"},

/*initial number of application threads deployed */
{core_ini_threads_nr, "4"}



}

#endif // !PROPERTIES_H_INCLUDED
