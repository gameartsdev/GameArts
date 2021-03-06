

#pragma once
#include <libethcore/Block.h>
#include <libethcore/LogEntry.h>
#include <libeventfilter/EventLogFilterParams.h>
namespace dev
{
namespace event
{
class EventLogFilter
{
public:
    using Ptr = std::shared_ptr<EventLogFilter>;

public:
    // constructor function
    EventLogFilter(
        EventLogFilterParams::Ptr _params, eth::BlockNumber _nextBlockToProcess, uint32_t _version)
      : m_params(_params),
        m_nextBlockToProcess(_nextBlockToProcess),
        m_channelProtocolVersion(_version)
    {}

public:
    // m_params
    EventLogFilterParams::Ptr getParams() const { return m_params; }
    // m_nextBlockToProcess
    eth::BlockNumber getNextBlockToProcess() const { return m_nextBlockToProcess; }
    // m_responseCallback
    std::function<bool(const std::string& _filterID, int32_t _result, const Json::Value& _logs)>
    getResponseCallback()
    {
        return m_responseCallback;
    }
    // m_sessionActive
    std::function<bool()> getSessionActiveCallback() { return m_isSessionActive; }

    // this filter pushed end
    bool pushCompleted() const { return m_nextBlockToProcess > m_params->getToBlock(); }

    // update m_nextBlockToProcess
    void updateNextBlockToProcess(eth::BlockNumber _nextBlockToProcess)
    {
        m_nextBlockToProcess = _nextBlockToProcess;
    }
    // set response call back
    void setResponseCallBack(
        std::function<bool(const std::string& _filterID, int32_t _result, const Json::Value& _logs)>
            _callback)
    {
        m_responseCallback = _callback;
    }

    void setCheckSessionActiveCallBack(std::function<bool()> _callback)
    {
        m_isSessionActive = _callback;
    }

    uint32_t getChannelProtocolVersion() const { return m_channelProtocolVersion; }

    void matches(eth::Block const& _block, Json::Value& _result);
    // filter individual log to see if the requirements are met
    bool matches(eth::LogEntry const& _log);

private:
    // event filter params generate from client request.
    EventLogFilterParams::Ptr m_params;
    // next block number to be processed.
    eth::BlockNumber m_nextBlockToProcess;
    // channel protocol version
    uint32_t m_channelProtocolVersion;
    // response callback function
    std::function<bool(const std::string& _filterID, int32_t _result, const Json::Value& _logs)>
        m_responseCallback;
    // connect active check function
    std::function<bool()> m_isSessionActive;
};

}  // namespace event
}  // namespace dev
