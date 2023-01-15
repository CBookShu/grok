#include "entity.h"

using namespace grok;

Entity Entity::instance;
Entity::~Entity()
{
    destroy();
}

void Entity::destroy()
{
    for (auto i : type_holders_)
    {
        delete i.second;
    }
    type_holders_.clear();
}

void Entity::Init()
{

}

Entity& Entity::GetEntity()
{
    return instance;
}

void Entity::Uinit()
{
    instance.destroy();
}