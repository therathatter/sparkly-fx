#include "stream.h"
#include "materials.h"
#include <Base/Interfaces.h>
#include <SDK/texture_group_names.h>

Stream::Ptr Stream::Clone(std::string&& new_name) const
{
    Stream::Ptr clone = std::make_shared<Stream>(std::move(new_name));
    for (auto& tweak : m_tweaks)
        clone->m_tweaks.emplace_back(std::move(tweak->Clone()));
    return clone;
}

const std::vector<std::shared_ptr<const Stream>>& Stream::GetPresets()
{
    static std::vector<Stream::ConstPtr> presets = MakePresets();
    return presets;
}

std::vector<Stream::ConstPtr> Stream::MakePresets()
{
    std::vector<Stream::ConstPtr> vec;
    
    // Player matte
    {
        Stream::Ptr matte = std::make_shared<Stream>("Player matte");

        auto entities = std::make_shared<EntityFilterTweak>();
        entities->render_effect = EntityFilterTweak::MaterialChoice::CUSTOM;
        entities->custom_material = CustomMaterial::GetMatte();
        entities->color_multiply = {0,1,0,1};
        entities->filter_choice = FilterChoice::WHITELIST;
        entities->filter_player = true;
        entities->filter_weapon = true;
        entities->filter_wearable = true;
        matte->m_tweaks.emplace_back(std::move(entities));

        auto fog = std::make_shared<FogTweak>();
        fog->fog_enabled = true;
        matte->m_tweaks.emplace_back(std::move(fog));

        auto misc = std::make_shared<MiscTweak>();
        misc->skybox_enabled = false;
        misc->misc_effects_enabled = false;
        // Particles are intentionally left enabled, so they may obscure the player matte
        matte->m_tweaks.emplace_back(std::move(misc));

        vec.emplace_back(std::move(matte));
    }
    // Depth fog
    {
        Stream::Ptr depth = std::make_shared<Stream>("Depth fog");

        auto entities = std::make_shared<EntityFilterTweak>();
        entities->render_effect = EntityFilterTweak::MaterialChoice::CUSTOM;
        entities->custom_material = CustomMaterial::GetSolid();
        entities->color_multiply = {0,0,0,1};
        depth->m_tweaks.emplace_back(std::move(entities));

        // Make all materials black
        auto materials = std::make_shared<MaterialTweak>();
        materials->filter_choice = FilterChoice::WHITELIST;
        materials->color_multiply = {0,0,0,1};
        for (size_t i = 0; i < MaterialTweak::TEXTURE_GROUPS.size(); ++i)
        {
            const char* group = MaterialTweak::TEXTURE_GROUPS[i];
            materials->groups[i] =  group == TEXTURE_GROUP_CLIENT_EFFECTS
                                    || group == TEXTURE_GROUP_WORLD
                                    || group == TEXTURE_GROUP_SKYBOX
                                    || group == TEXTURE_GROUP_DECAL;
        }
        depth->m_tweaks.push_back(std::move(materials));
        
        auto fog = std::make_shared<FogTweak>();
        fog->fog_enabled = true;
        fog->fog_color = {1,1,1};
        fog->fog_start = 2048;
        depth->m_tweaks.push_back(std::move(fog));

        auto misc = std::make_shared<MiscTweak>();
        misc->decals_enabled = false;
        misc->shadows_enabled = false;
        misc->skybox_enabled = false;
        misc->particles_enabled = false;
        misc->misc_effects_enabled = false;
        depth->m_tweaks.push_back(std::move(misc));
        
        vec.emplace_back(std::move(depth));
    }

    return vec;
}