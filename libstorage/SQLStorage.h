 
/** @file SQLStorage.h
 *  @author ancelmo
 *  @date 20190326
 */

#pragma once

#include "Storage.h"
#include <json/json.h>
#include <libchannelserver/ChannelRPCServer.h>
#include <libdevcore/FixedHash.h>

namespace dev
{
namespace storage
{
class SQLStorage : public Storage
{
public:
    typedef std::shared_ptr<SQLStorage> Ptr;

    SQLStorage();
    virtual ~SQLStorage(){};

    Entries::Ptr select(int64_t num, TableInfo::Ptr tableInfo, const std::string& key,
        Condition::Ptr condition) override;
    size_t commit(int64_t num, const std::vector<TableData::Ptr>& datas) override;
    TableData::Ptr selectTableDataByNum(
        int64_t num, TableInfo::Ptr tableInfo, uint64_t start, uint32_t counts);
    bool onlyCommitDirty() override { return true; }

    virtual void setTopic(const std::string& topic);
    virtual void setChannelRPCServer(dev::ChannelRPCServer::Ptr channelRPCServer);
    virtual void setMaxRetry(int maxRetry);

    virtual void setFatalHandler(std::function<void(std::exception&)> fatalHandler)
    {
        m_fatalHandler = fatalHandler;
    }
    // seconds
    void setTimeout(size_t timeout = 10) { m_timeout = timeout * 1000; }

private:
    Json::Value requestDB(const Json::Value& value);

    std::function<void(std::exception&)> m_fatalHandler;

    std::string m_topic;
    dev::ChannelRPCServer::Ptr m_channelRPCServer;
    int m_maxRetry = 0;
    size_t m_timeout = 10 * 1000;  // timeout by ms
};

}  // namespace storage

}  // namespace dev
