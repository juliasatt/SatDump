#include "core/plugin.h"
#include "logger.h"
#include "core/module.h"

#include "iq/module_iq_conversion.h"
#include "iq/module_iq_re.h"

class IQManipulationSupport : public satdump::Plugin
{
public:
    std::string getID()
    {
        return "iq_manipulation_support";
    }

    void init()
    {
        satdump::eventBus->register_handler<RegisterModulesEvent>(registerPluginsHandler);
    }

    static void registerPluginsHandler(const RegisterModulesEvent &evt)
    {
        REGISTER_MODULE_EXTERNAL(evt.modules_registry, iq_conversion::IQConversionModule);
        REGISTER_MODULE_EXTERNAL(evt.modules_registry, iq_re::IQREModule);
    }
};

PLUGIN_LOADER(IQManipulationSupport)
