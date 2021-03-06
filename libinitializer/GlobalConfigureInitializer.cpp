

#include "GlobalConfigureInitializer.h"
#include "include/BuildInfo.h"
#include "libsecurity/KeyCenter.h"
#include <libethcore/EVMSchedule.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace dev;
using namespace dev::initializer;

DEV_SIMPLE_EXCEPTION(UnknowSupportVersion);

uint32_t dev::initializer::getVersionNumber(const string& _version)
{
    // 0XMNNPPTS, M=MAJOR N=MINOR P=PATCH T=TWEAK S=STATUS
    vector<string> versions;
    uint32_t versionNumber = 0;
    boost::split(versions, _version, boost::is_any_of("."));
    if (versions.size() != 3)
    {
        BOOST_THROW_EXCEPTION(UnknowSupportVersion() << errinfo_comment(_version));
    }
    try
    {
        for (size_t i = 0; i < versions.size(); ++i)
        {
            versionNumber += boost::lexical_cast<uint32_t>(versions[i]);
            versionNumber <<= 8;
        }
    }
    catch (const boost::bad_lexical_cast& e)
    {
        INITIALIZER_LOG(ERROR) << LOG_KV("what", boost::diagnostic_information(e));
        BOOST_THROW_EXCEPTION(UnknowSupportVersion() << errinfo_comment(_version));
    }
    return versionNumber;
}

void dev::initializer::initGlobalConfig(const boost::property_tree::ptree& _pt)
{
    /// default version is RC1
    string version = _pt.get<string>("compatibility.supported_version", "2.0.0-rc1");
    uint32_t versionNumber = 0;
    if (dev::stringCmpIgnoreCase(version, "2.0.0-rc1") == 0)
    {
        g_BCOSConfig.setSupportedVersion(version, RC1_VERSION);
    }
    else if (dev::stringCmpIgnoreCase(version, "2.0.0-rc2") == 0)
    {
        g_BCOSConfig.setSupportedVersion(version, RC2_VERSION);
    }
    else if (dev::stringCmpIgnoreCase(version, "2.0.0-rc3") == 0)
    {
        g_BCOSConfig.setSupportedVersion(version, RC3_VERSION);
    }
    else
    {
        versionNumber = getVersionNumber(version);
        g_BCOSConfig.setSupportedVersion(version, static_cast<VERSION>(versionNumber));
    }

    // set evmSchedule
    if (g_BCOSConfig.version() <= getVersionNumber("2.0.0"))
    {
        g_BCOSConfig.setEVMSchedule(dev::eth::GameArtSchedule);
    }
    else
    {
        g_BCOSConfig.setEVMSchedule(dev::eth::GameArtScheduleV2);
    }

    g_BCOSConfig.binaryInfo.version = GAME_ART_PROJECT_VERSION;
    g_BCOSConfig.binaryInfo.buildTime = GAME_ART_BUILD_TIME;
    g_BCOSConfig.binaryInfo.buildInfo =
        string(GAME_ART_BUILD_PLATFORM) + "/" + string(GAME_ART_BUILD_TYPE);
    g_BCOSConfig.binaryInfo.gitBranch = GAME_ART_BUILD_BRANCH;
    g_BCOSConfig.binaryInfo.gitCommitHash = GAME_ART_COMMIT_HASH;

    string sectionName = "data_secure";
    if (_pt.get_child_optional("storage_security"))
    {
        sectionName = "storage_security";
    }

    g_BCOSConfig.diskEncryption.enable = _pt.get<bool>(sectionName + ".enable", false);
    g_BCOSConfig.diskEncryption.keyCenterIP = _pt.get<string>(sectionName + ".key_manager_ip", "");
    g_BCOSConfig.diskEncryption.keyCenterPort =
        _pt.get<int>(sectionName + ".key_manager_port", 20000);
    if (!isValidPort(g_BCOSConfig.diskEncryption.keyCenterPort))
    {
        BOOST_THROW_EXCEPTION(
            InvalidPort() << errinfo_comment("P2PInitializer:  initConfig for storage_security "
                                             "failed! Invalid key_manange_port!"));
    }


    /// compress related option, default enable
    bool enableCompress = _pt.get<bool>("p2p.enable_compress", true);
    g_BCOSConfig.setCompress(enableCompress);

    /// init version
    int64_t chainId = _pt.get<int64_t>("chain.id", 1);
    if (chainId < 0)
    {
        BOOST_THROW_EXCEPTION(
            ForbidNegativeValue() << errinfo_comment("Please set chain.id to positive!"));
    }
    g_BCOSConfig.setChainId(chainId);

    if (g_BCOSConfig.diskEncryption.enable)
    {
        auto cipherDataKey = _pt.get<string>(sectionName + ".cipher_data_key", "");
        if (cipherDataKey.empty())
        {
            BOOST_THROW_EXCEPTION(
                MissingField() << errinfo_comment("Please provide cipher_data_key!"));
        }
        KeyCenter keyClient;
        keyClient.setIpPort(
            g_BCOSConfig.diskEncryption.keyCenterIP, g_BCOSConfig.diskEncryption.keyCenterPort);
        g_BCOSConfig.diskEncryption.cipherDataKey = cipherDataKey;
        g_BCOSConfig.diskEncryption.dataKey = asString(keyClient.getDataKey(cipherDataKey));
        INITIALIZER_LOG(INFO) << LOG_BADGE("initKeyManager")
                              << LOG_KV("url.IP", g_BCOSConfig.diskEncryption.keyCenterIP)
                              << LOG_KV("url.port",
                                     to_string(g_BCOSConfig.diskEncryption.keyCenterPort));
    }

    INITIALIZER_LOG(INFO) << LOG_BADGE("initGlobalConfig")
                          << LOG_KV("enableCompress", g_BCOSConfig.compressEnabled())
                          << LOG_KV("compatibilityVersion", version)
                          << LOG_KV("versionNumber", g_BCOSConfig.version())
                          << LOG_KV("chainId", g_BCOSConfig.chainId());
}

void dev::version()
{
    std::cout << "GAME-ART Version : " << GAME_ART_PROJECT_VERSION << std::endl;
    std::cout << "Build Time         : " << GAME_ART_BUILD_TIME << std::endl;
    std::cout << "Build Type         : " << GAME_ART_BUILD_PLATFORM << "/"
              << GAME_ART_BUILD_TYPE << std::endl;
    std::cout << "Git Branch         : " << GAME_ART_BUILD_BRANCH << std::endl;
    std::cout << "Git Commit Hash    : " << GAME_ART_COMMIT_HASH << std::endl;
}
