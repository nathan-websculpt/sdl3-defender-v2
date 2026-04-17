#include <SDL3/SDL.h>
#include <charconv>
#include <core/startup/app_bootstrap.h>
#include <filesystem>
#include <iostream>
#include <string_view>
namespace
{

std::filesystem::path pathFromUtf8(const char* utf8Path)
{
    if (!utf8Path)
    {
        return {};
    }

    std::u8string utf8Bytes;
    for (const char* it = utf8Path; *it != '\0'; ++it)
    {
        utf8Bytes.push_back(static_cast<char8_t>(static_cast<unsigned char>(*it)));
    }

    return std::filesystem::path(utf8Bytes);
}

bool parseSeedArg(std::string_view arg, std::uint64_t& seedOut)
{
    constexpr std::string_view prefix = "--seed=";
    if (!arg.starts_with(prefix))
    {
        return false;
    }

    const std::string_view value = arg.substr(prefix.size());
    if (value.empty())
    {
        return false;
    }

    std::uint64_t parsed = 0;
    const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size())
    {
        return false;
    }

    seedOut = parsed;
    return true;
}

} // namespace

void AppStartup::applyBasePathWorkingDirectory()
{
    try
    {
        const char* basePathUtf8 = SDL_GetBasePath();
        if (!basePathUtf8)
        {
            std::cerr << "Failed to get SDL base path: " << SDL_GetError() << std::endl;
        }
        else
        {
            std::filesystem::path basePath = pathFromUtf8(basePathUtf8);
            std::filesystem::current_path(basePath);
        }
        std::cout << "Working directory set to: " << std::filesystem::current_path().string()
                  << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to set working directory: " << e.what() << std::endl;
    }
}

AppStartup::LaunchSeedParseResult AppStartup::parseLaunchSeed(int argc, char* argv[])
{
    std::optional<std::uint64_t> requestedSeed;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i] ? std::string_view(argv[i]) : std::string_view{};
        if (!arg.starts_with("--seed="))
        {
            continue;
        }

        std::uint64_t parsedSeed = 0;
        if (!parseSeedArg(arg, parsedSeed))
        {
            return LaunchSeedParseResult{false, std::nullopt, std::string(arg)};
        }
        requestedSeed = parsedSeed;
    }

    return LaunchSeedParseResult{true, requestedSeed, {}};
}
