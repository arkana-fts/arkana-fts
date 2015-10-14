
#include <algorithm>

#include "ChannelManager.h"
#include "channel.h"
#include "client.h"
#include "db.h"
#include "config.h"

using namespace FTS;
using namespace FTSSrv2;

FTSSrv2::ChannelManager::ChannelManager()
{
    m_lpChannels.clear();
}

FTSSrv2::ChannelManager::~ChannelManager()
{
    this->saveChannels( );

    for( const auto& i : m_lpChannels ) {
        delete i;
    }

    m_lpChannels.clear();
}

int FTSSrv2::ChannelManager::init()
{
    // First, load all the channels from the database.
    if(ERR_OK != loadChannels()) {
        return -1;
    }

    // Then, check if the main channel is existing.
    if(getDefaultChannel() == nullptr) {
        // If not, create it with Pompei2 as admin.
        // We do create it manually here because it's a special case.
        m_lpChannels.push_back(new FTSSrv2::Channel(-1, true,
                                                      DSRV_DEFAULT_CHANNEL_NAME,
                                                      DSRV_DEFAULT_CHANNEL_MOTTO,
                                                      DSRV_DEFAULT_CHANNEL_ADMIN));

    } else if(getDefaultChannel()->getAdmin() != DSRV_DEFAULT_CHANNEL_ADMIN) {
        // Or if somehow another admin is entered in it, set it to the default admin!
        getDefaultChannel()->setAdmin(DSRV_DEFAULT_CHANNEL_ADMIN);
    }

    // Same for the dev's channel.
    if(findChannel(DSRV_DEVS_CHANNEL_NAME) == nullptr) {
        // If not, create it with Pompei2 as admin.
        // We do create it manually here because it's a special case.
        m_lpChannels.push_back(new FTSSrv2::Channel(-1, true,
                                                      DSRV_DEVS_CHANNEL_NAME,
                                                      DSRV_DEVS_CHANNEL_MOTTO,
                                                      DSRV_DEVS_CHANNEL_ADMIN));

    } else if(findChannel(DSRV_DEVS_CHANNEL_NAME)->getAdmin() != DSRV_DEVS_CHANNEL_ADMIN) {
        // Or if somehow another admin is entered in it, set it to the default admin!
        findChannel(DSRV_DEVS_CHANNEL_NAME)->setAdmin(DSRV_DEVS_CHANNEL_ADMIN);
    }

    return ERR_OK;
}

FTSSrv2::ChannelManager *FTSSrv2::ChannelManager::getManager()
{
    return Singleton::getSingletonPtr();
}

void FTSSrv2::ChannelManager::deinit()
{
    delete Singleton::getSingletonPtr();
}

int FTSSrv2::ChannelManager::loadChannels(void)
{
    MYSQL_RES *pRes = nullptr;
    MYSQL_ROW pRow = nullptr;

    // Do the query to get the field.
    String sQuery = "SELECT  `"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_ID)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_PUBLIC)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_NAME)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_MOTTO)+"`"
                            ",`"+DataBase::getUniqueDB()->TblChansField(DSRV_TBL_CHANS_ADMIN)+"`"
                     " FROM `" DSRV_TBL_CHANS "`";

    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        DataBase::getUniqueDB()->free(pRes);
        return -1;
    }

    // Invalid record ? forget about it!
    if(pRes == nullptr || mysql_num_fields(pRes) < 5) {
        DataBase::getUniqueDB()->free(pRes);
        return -2;
    }

    // Create every single channel.
    while(nullptr != (pRow = mysql_fetch_row(pRes))) {
        int iChannelID = atoi(pRow[0]);
        bool bPublic = (pRow[1] == nullptr ? false : (pRow[1][0] == '0' ? false : true));
        String sChanName = pRow[2];
        String sChanMotto = pRow[3];
        String sChanAdmin = pRow[4];

        FTSSrv2::Channel *pChan = new FTSSrv2::Channel(iChannelID, bPublic, sChanName, sChanMotto, sChanAdmin);
        m_lpChannels.push_back(pChan);
    }

    DataBase::getUniqueDB()->free(pRes);

    // Now read all channel operators.
    sQuery = "SELECT  `"+DataBase::getUniqueDB()->TblChanOpsField(DSRV_VIEW_CHANOPS_NICK)+"`"
                    ",`"+DataBase::getUniqueDB()->TblChanOpsField(DSRV_VIEW_CHANOPS_CHAN)+"`"
             " FROM `" DSRV_VIEW_CHANOPS "`";
    if(!DataBase::getUniqueDB()->query(pRes, sQuery)) {
        DataBase::getUniqueDB()->free(pRes);
        return -3;
    }

    // Invalid record ? forget about it!
    if(pRes == nullptr || mysql_num_fields(pRes) < 2) {
        DataBase::getUniqueDB()->free(pRes);
        return -4;
    }

    // Setup every operator<->channel connection.
    // But first just put all assocs. in a list because we need to free the DB.
    std::list<std::pair<FTSSrv2::Channel *, String> > operators;
    while(nullptr != (pRow = mysql_fetch_row(pRes))) {
        FTSSrv2::Channel *pChan = this->findChannel(pRow[1]);

        if(!pChan)
            continue;

        operators.push_back(std::make_pair(pChan, pRow[0]));
    }

    DataBase::getUniqueDB()->free(pRes);

    // Now we execute that action (only now as the DB result needs to be freed).
    for( auto& i : operators ) {
        i.first->op(i.second, true);
    }

    return ERR_OK;
}

int FTSSrv2::ChannelManager::saveChannels( void )
{
    Lock l(m_mutex);
    for( const auto& i : m_lpChannels ) {
        i->save();
    }

    return ERR_OK;
}

FTSSrv2::Channel *FTSSrv2::ChannelManager::createChannel(const String & in_sName, const Client *in_pCreater, bool in_bPublic)
{
    // Every user can only create a limited amount of channels!
    if(this->countUserChannels(in_pCreater->getNick()) >= DSRV_MAX_CHANS_PER_USER) {
        return nullptr;
    }

    Lock l(m_mutex);
    FTSSrv2::Channel *pChannel = new FTSSrv2::Channel(-1, in_bPublic, in_sName,
                                      DSRV_DEFAULT_MOTTO,
                                      in_pCreater->getNick());

    m_lpChannels.push_back(pChannel);
    pChannel->save(); // Update the database right now!

    return pChannel;
}

int FTSSrv2::ChannelManager::removeChannel(FTSSrv2::Channel *out_pChannel, const String &in_sWhoWantsIt)
{
    Lock l(m_mutex);
    for(const auto& i : m_lpChannels) {
        if(i == out_pChannel) {
            // Found! remove it from DB and manager, if we have the rights!
            if(ERR_OK == out_pChannel->destroyDB(in_sWhoWantsIt)) {
                m_lpChannels.remove(i);
                // The order here is important, as deleting the channel might kick players, that will be locking the mutex.
                SAFE_DELETE(out_pChannel);
                return ERR_OK;
            } else {
                // Found, but no right to remove it.
                return -2;
            }
        }
    }

    // Not found.
    return -1;
}

std::list<FTSSrv2::Channel *> FTSSrv2::ChannelManager::getPublicChannels()
{
    std::list<FTSSrv2::Channel *>lpPubChannels;

    Lock l(m_mutex);
    for( const auto& i : m_lpChannels ) {
        if( i->isPublic( ) ) {
            lpPubChannels.push_back(i);
        }
    }

    return lpPubChannels;
}

int FTSSrv2::ChannelManager::joinChannel( FTSSrv2::Channel *out_pChannel, Client *out_pClient )
{
    if(out_pChannel == nullptr)
        return -1;

    FTSSrv2::Channel *pOldChan = out_pClient->getMyChannel();

    Lock l(m_mutex);
    // Leave the old channel.
    if(pOldChan) {
        pOldChan->quit(out_pClient);
    }

    // Join the new channel.
    out_pChannel->join(out_pClient);

    return ERR_OK;
}

FTSSrv2::Channel *FTSSrv2::ChannelManager::findChannel(const String & in_sName)
{
    Lock l(m_mutex);
    for(const auto& i : m_lpChannels) {
        // "Pompei2's ChanNel" is the same as "pOmpei2's chAnnEl"
        if(i->getName().ieq(in_sName)) {
            return i;
        }
    }

    return nullptr;
}

std::uint32_t FTSSrv2::ChannelManager::countUserChannels(const String &in_sUserName)
{
    Lock l(m_mutex);
    std::uint32_t nChans = (std::uint32_t) std::count_if( std::begin( m_lpChannels ), std::end( m_lpChannels ), [in_sUserName] ( Channel* pChan ){ return pChan->getAdmin().ieq( in_sUserName); } );
    return nChans;
}

std::list<String> FTSSrv2::ChannelManager::getUserChannels(const String &in_sUserName)
{
    std::list<String> sChans;

    Lock l(m_mutex);
    for(const auto& i : m_lpChannels) {
        if(i->getAdmin().ieq(in_sUserName)) {
            sChans.push_back(i->getName());
        }
    }

    return sChans;
}

FTSSrv2::Channel *FTSSrv2::ChannelManager::getDefaultChannel(void)
{
    return this->findChannel(DSRV_DEFAULT_CHANNEL_NAME);
}

