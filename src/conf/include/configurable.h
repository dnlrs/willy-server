#ifndef CONFIGURABLE_H_INCLUDED
#define CONFIGURABLE_H_INCLUDED

#include "cfg.h"
#include <memory>

namespace cfg
{
    class configurable
    {
    public:
        virtual bool update_configuration(
                std::shared_ptr<cfg::configuration> new_cfg) = 0;
    };
}


#endif // !CONFIGURABLE_H_INCLUDED
