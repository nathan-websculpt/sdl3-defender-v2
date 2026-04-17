#pragma once

#include <cstdint>
#include <optional>
#include <string>
namespace AppStartup
{

struct LaunchSeedParseResult
{
    bool valid = true;
    std::optional<std::uint64_t> requestedSeed;
    std::string invalidArgument;
};

void applyBasePathWorkingDirectory();
LaunchSeedParseResult parseLaunchSeed(int argc, char* argv[]);

} // namespace AppStartup
