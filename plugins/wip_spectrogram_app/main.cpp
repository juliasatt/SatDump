#include "core/plugin.h"
#include "explorer/explorer.h"
#include "spectrogram.h"

class SpectrogramAppPlugin : public satdump::Plugin
{
public:
    std::string getID() { return "wipspectrogram_app"; }

    void init()
    {
        // TODOREWORK maybe a way to call up/init a handler?
        satdump::eventBus->register_handler<satdump::explorer::RenderLoadMenuElementsEvent>(renderExplorerLoaderButton);
    }

    static void renderExplorerLoaderButton(const satdump::explorer::RenderLoadMenuElementsEvent &evt)
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Spectrogram App"))
                    evt.master_handler->addSubHandler(std::make_shared<satdump::SpectrogramHandler>());
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
    }
};

PLUGIN_LOADER(SpectrogramAppPlugin)
