/**
 * \file ModelManager.cpp
 * \author Pompei2
 * \date 12 August 2010
 * \brief This file contains the ModelManager class.
 **/

#include "ModelManager.h"
#include "HardwareModel.h"
#include "ModelInstance.h"

#include "3d/Shader.h"
#include "logging/logger.h"

#include "dLib/dString/dString.h"
#include "dLib/dArchive/dArchive.h"

#define D_MODELS_DIRNAME "Models"

FTS::ModelManager::ModelManager()
    : ErrorModelName("Error")
{
    FTSMSGDBG("Creating hw model manager", 2);

    // Create the error model that will (hopefully) never fail.
    m_mHardwareModels[ErrorModelName] = std::shared_ptr<HardwareModel>(new HardwareModel(ErrorModelName));

    FTSMSGDBG("Created hw model manager", 2);
}

FTS::ModelManager::~ModelManager()
{
    FTSMSGDBG("Destroying hw model manager", 2);

    // Here, we have the responsability to destroy them all!
    this->removeAllModels();

    FTSMSGDBG("Destroyed hw model manager", 2);
}

FTS::ModelInstance* FTS::ModelManager::createInstance(const FTS::String& in_sName)
{
    return new ModelInstance(this->getOrLoad(in_sName));
}

void FTS::ModelManager::addModel(const FTS::String& in_sName)
{
    this->getOrLoad(in_sName);
}

std::shared_ptr<FTS::HardwareModel> FTS::ModelManager::getOrLoad(const FTS::String& in_sName)
{
    auto i = m_mHardwareModels.find(in_sName);
    if(i != m_mHardwareModels.end())
        return i->second;

    // Not loaded yet, then load it!
    try {
        // Check if the name is either just a "name" (of a model in or models dir) ...
        Path toOpen = Path::datadir(D_MODELS_DIRNAME) + Path(in_sName + ".ftsmdl");
        if(!FileUtils::exists(toOpen)) {
            // Next, try to open it without the .ftsmdl extension (in the case it is a directory)
            toOpen = Path::datadir(D_MODELS_DIRNAME) + Path(in_sName);
            if(!FileUtils::exists(toOpen)) {
                // Last chance is that we got a full pathname already:
                toOpen = in_sName;
            }
        }

        // Load the model out of the archive.
        Archive::Ptr pArch  = Archive::Ptr(Archive::loadArchive(toOpen));
        m_mHardwareModels[in_sName] = std::shared_ptr<HardwareModel>(new HardwareModel(in_sName, *pArch));

        // And here the archive gets closed again, automagic :)
    } catch(const FTS::LoggableException& e) {
        e.show();

        // If it hasn't been loaded successfully, we still add it to the map as being
        // the error texture. To avoid hundreds of reload trials.
        m_mHardwareModels[in_sName] = this->getErrorModel();
    } catch(const std::exception& e) {
        FTS18N("CorruptData", MsgType::Error, in_sName, e.what());

        // same here.
        m_mHardwareModels[in_sName] = this->getErrorModel();
    }

    return m_mHardwareModels[in_sName];
}

void FTS::ModelManager::removeModel(const String& in_sName)
{
    if(in_sName != ErrorModelName)
        m_mHardwareModels.erase(in_sName);
}

void FTS::ModelManager::removeAllModels()
{
    if(m_mHardwareModels.size() > 1) {
        String sWarning = "The following hardware models haven't been unloaded:\n";
        for(auto model = m_mHardwareModels.begin() ; model != m_mHardwareModels.end() ; ++model) {
            sWarning += "    -> " + model->first + " (" + String::nr(model->second.use_count()) + " references)\n";
        }
        FTSMSGDBG(sWarning, 2);
    }

    m_mHardwareModels.clear();
}

std::shared_ptr<FTS::HardwareModel> FTS::ModelManager::getErrorModel()
{
    return m_mHardwareModels[ErrorModelName];
}
