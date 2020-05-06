

#include "TxsParallelParser.h"
#include "Exceptions.h"
#include <tbb/parallel_for.h>

namespace dev
{
namespace eth
{
bytes TxsParallelParser::encode(std::shared_ptr<Transactions> _txs)
{
    Offset_t txNum = _txs->size();
    if (txNum == 0)
        return bytes();

    std::vector<Offset_t> offsets(txNum + 1, 0);
    std::vector<bytes> txRLPs(txNum, bytes());

    // encode tx and caculate offset
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, txNum), [&](const tbb::blocked_range<size_t>& _r) {
            for (Offset_t i = _r.begin(); i < _r.end(); ++i)
            {
                bytes txByte = (*_txs)[i]->rlp(WithSignature);

                // record bytes size in offsets for caculating real offset
                offsets[i + 1] = txByte.size();

                // record bytes in txRLPs for caculating all transaction bytes
                txRLPs[i] = txByte;
            }
        });

    // caculate real offset
    for (size_t i = 0; i < txNum; ++i)
    {
        offsets[i + 1] += offsets[i];
    }

    // encode according with this protocol
    bytes ret;
    ret += toBytes(Offset_t(txNum));
    for (size_t i = 0; i < offsets.size(); ++i)
        ret += toBytes(offsets[i]);

    for (size_t i = 0; i < txNum; ++i)
        ret += txRLPs[i];

    // std::cout << "tx encode:" << toHex(ret) << std::endl;
    return ret;
}

bytes TxsParallelParser::encode(std::vector<bytes> const& _txs)
{
    Offset_t txNum = _txs.size();
    if (txNum == 0)
        return bytes();

    bytes txBytes;
    std::vector<Offset_t> offsets(txNum + 1, 0);
    Offset_t offset = 0;

    // encode tx and caculate offset
    for (Offset_t i = 0; i < txNum; ++i)
    {
        bytes const& txByte = _txs[i];
        offsets[i] = offset;
        offset += txByte.size();
        txBytes += txByte;
    }
    offsets[txNum] = offset;  // write the end

    // encode according with this protocol
    bytes ret;
    ret += toBytes(Offset_t(txNum));
    for (size_t i = 0; i < offsets.size(); ++i)
        ret += toBytes(offsets[i]);

    ret += txBytes;

    // std::cout << "tx encode:" << toHex(ret) << std::endl;
    return ret;
}

inline void throwInvalidBlockFormat(const std::string& _reason)
{
    BOOST_THROW_EXCEPTION(
        InvalidBlockFormat() << errinfo_comment("Block transactions bytes is invalid: " + _reason));
}

// parallel decode transactions
void TxsParallelParser::decode(std::shared_ptr<Transactions> _txs, bytesConstRef _bytes,
    CheckTransaction _checkSig, bool _withHash)
{
    try
    {
        size_t bytesSize = _bytes.size();
        if (bytesSize == 0)
            return;
        // std::cout << "tx decode:" << toHex(_bytes) << std::endl;
        Offset_t txNum = fromBytes(_bytes.cropped(0));
        // check txNum
        size_t objectStart = sizeof(Offset_t) * (txNum + 2);
        if (objectStart >= bytesSize)
        {
            throwInvalidBlockFormat("objectStart >= bytesSize");
        }

        // Get offsets space
        vector_ref<Offset_t> offsets(reinterpret_cast<Offset_t*>(const_cast<unsigned char*>(
                                         _bytes.cropped(sizeof(Offset_t)).data())),
            txNum + 1);
        _txs->resize(txNum);

        // Get objects space
        bytesConstRef txBytes = _bytes.cropped(sizeof(Offset_t) * (txNum + 2));

        // check objects space
        if (offsets.size() == 0 || txBytes.size() == 0)
            throwInvalidBlockFormat("offsets.size() == 0 || txBytes.size() == 0");

        // parallel decoding
        size_t maxOffset = bytesSize - objectStart - 1;
        try
        {
            tbb::parallel_for(tbb::blocked_range<Offset_t>(0, txNum),
                [&](const tbb::blocked_range<Offset_t>& _r) {
                    for (Offset_t i = _r.begin(); i != _r.end(); ++i)
                    {
                        Offset_t offset = offsets[i];
                        Offset_t size = offsets[i + 1] - offsets[i];

                        if (offset > maxOffset)
                            throwInvalidBlockFormat("offset > maxOffset");

                        (*_txs)[i] = std::make_shared<Transaction>();
                        (*_txs)[i]->decode(txBytes.cropped(offset, size), _checkSig);
                        if (_withHash)
                        {
                            // cache the sha3
                            // Note: can't calculate sha3 with sha3(txBytes.cropped(offset, size))
                            // directly
                            //       considering that some cases the encodedData is not
                            //       equal to txBytes.cropped(offset, size)
                            (*_txs)[i]->sha3();
                        } /*
                         LOG(DEBUG) << LOG_BADGE("DECODE") << LOG_DESC("decode tx:") << LOG_KV("i",
                         i)
                                    << LOG_KV("offset", offset)
                                    << LOG_KV("code", toHex(txBytes.cropped(offset, size)));
                                    */
                    }
                });
        }
        catch (...)
        {
            throw;
        }
    }
    catch (Exception& e)
    {
        throwInvalidBlockFormat(e.what());
    }
}

}  // namespace eth
}  // namespace dev
