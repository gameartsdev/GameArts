

#pragma once
#include <libdevcore/FixedHash.h>
#include <libethcore/Protocol.h>
#include <libnetwork/Common.h>
#include <memory>

namespace dev
{
namespace p2p
{
class P2PMessage : public dev::network::Message
{
public:
    typedef std::shared_ptr<P2PMessage> Ptr;
    /// m_length: 4bytes
    /// pid + gid: 2bytes
    /// packetType: 2bytes
    /// seq: 4 bytes
    const static size_t HEADER_LENGTH = 12;
    const static size_t MAX_LENGTH = 1024 * 1024;  ///< The maximum length of data is 1M.

    P2PMessage() { m_buffer = std::make_shared<bytes>(); }

    virtual ~P2PMessage() {}

    virtual uint32_t length() override { return m_length; }

    virtual PROTOCOL_ID protocolID() { return m_protocolID; }
    virtual void setProtocolID(PROTOCOL_ID _protocolID) { setField(m_protocolID, _protocolID); }
    virtual PACKET_TYPE packetType() { return m_packetType; }
    virtual void setPacketType(PACKET_TYPE _packetType) { setField(m_packetType, _packetType); }

    virtual uint32_t seq() override { return m_seq; }
    virtual void setSeq(uint32_t _seq) { setField(m_seq, _seq); }

    virtual std::shared_ptr<bytes> buffer() { return m_buffer; }
    virtual void setBuffer(std::shared_ptr<bytes> _buffer)
    {
        m_buffer.reset();
        m_buffer = _buffer;
        /// update the length
        m_length = HEADER_LENGTH + m_buffer->size();
        m_dirty = true;
    }

    /// to compatible with RC1 even if m_protocolID is extended to int32_t
    /// attention:
    /// this logic is only used in AMOP
    /// the response packet in RC1 is -m_protocolID, but RC2 modifies m_protocolID to int32_t
    /// to make all the RC1 response packet is positive
    /// so we need to determine the packet is the response packet or not according to 16th
    /// binary number of the packet is 1 or 0
    virtual bool isRequestPacket() override { return !((m_protocolID & 0x8000) == 0x8000); }

    virtual void encode(bytes& buffer) override;

    /// < If the decoding is successful, the length of the decoded data is returned; otherwise, 0 is
    /// returned.
    virtual ssize_t decode(const byte* buffer, size_t size) override;

    /// update m_dirty according to updatedData
    template <class T>
    void setField(T& _originValue, T const& _newValue)
    {
        if (_originValue != _newValue)
        {
            _originValue = _newValue;
            m_dirty = true;
        }
    }
    bool dirty() const { return m_dirty; }

    virtual uint32_t deliveredLength() { return m_length; }

protected:
    uint32_t m_length = 0;            ///< m_length = HEADER_LENGTH + length(m_buffer)
    PROTOCOL_ID m_protocolID = 0;     ///< message type, the first two bytes of information, when
                                      ///< greater than 0 is the ID of the request package.
    PACKET_TYPE m_packetType = 0;     ///< message sub type, the second two bytes of information
    uint32_t m_seq = 0;               ///< the message identify
    std::shared_ptr<bytes> m_buffer;  ///< message data
    bool m_dirty = true;
};
enum AMOPPacketType
{
    SendTopicSeq = 1,
    RequestTopics = 2,
    SendTopics = 3,
};
}  // namespace p2p
}  // namespace dev
