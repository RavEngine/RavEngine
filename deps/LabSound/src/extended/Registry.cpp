
#include "LabSound/extended/Registry.h"

#include <cstdio>
#include <memory>
#include <map>
#include <mutex>


void LabSoundRegistryInit(lab::NodeRegistry&);

namespace lab {

std::once_flag instance_flag;
NodeRegistry& NodeRegistry::Instance()
{
    static NodeRegistry s_registry;
    std::call_once(instance_flag, []() { LabSoundRegistryInit(s_registry); });
    return s_registry;
}

struct NodeDescriptor
{
    std::string name;
    CreateNodeFn c;
    DeleteNodeFn d;
};

struct NodeRegistry::Detail
{
    std::map<std::string, NodeDescriptor> descriptors;
};


NodeRegistry::NodeRegistry() :
_detail(new Detail())
{
}

NodeRegistry::~NodeRegistry()
{
    delete _detail;
}

bool NodeRegistry::Register(char const* const name, CreateNodeFn c, DeleteNodeFn d)
{
    printf("Registering %s\n", name);
    _detail->descriptors[name] = { name, c, d };
    return true;
}

std::vector<std::string> NodeRegistry::Names() const
{
    std::vector<std::string> names;
    for (const auto& i : _detail->descriptors)
        names.push_back(i.second.name);

    return names;
}

lab::AudioNode* NodeRegistry::Create(const std::string& n, lab::AudioContext& ac)
{
    auto i = _detail->descriptors.find(n);
    if (i == _detail->descriptors.end())
        return nullptr;

    return i->second.c(ac);
}



} // lab
