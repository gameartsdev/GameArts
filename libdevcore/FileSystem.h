

#pragma once

#include <boost/filesystem.hpp>
#include <string>

namespace dev
{
/// Sets the data dir for the default ("ethereum") prefix.
void setDataDir(boost::filesystem::path const& _dir);
/// @returns the path for user data.
boost::filesystem::path getDataDir(std::string _prefix = "game-art-data");
boost::filesystem::path getLedgerDir(
    std::string ledger_name, std::string data_dir = "game-art-data");
/// @returns the default path for user data, ignoring the one set by `setDataDir`.
boost::filesystem::path getDefaultDataDir(std::string _prefix = "game-art-data");
/// Sets the ipc socket dir
void setIpcPath(boost::filesystem::path const& _ipcPath);
/// @returns the ipc path (default is DataDir)
boost::filesystem::path getIpcPath();

/// @returns a new path whose file name is suffixed with the given suffix.
boost::filesystem::path appendToFilename(
    boost::filesystem::path const& _orig, std::string const& _suffix);

}  // namespace dev
