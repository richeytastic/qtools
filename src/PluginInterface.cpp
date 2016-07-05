#include "PluginInterface.h"
using QTools::PluginInterface;

QStringList PluginInterface::getDisplayNames() const
{
    return QStringList() << this->getDisplayName();
}   // end getDisplayNames
