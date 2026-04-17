#include <iostream>
#include <optional>

#include <core/game.h>
#include <core/platform.h>
#include <core/startup/app_bootstrap.h>

// void* operator new(std::size_t size) {
//     std::cout << "Allocating " << size << " bytes\n";
//     return malloc(size);
// }
// void operator delete(void* ptr) noexcept {
//     std::cout << "Freeing memory\n";
//     free(ptr);
// }

int main(int argc, char* argv[]) {
    AppStartup::applyBasePathWorkingDirectory();

    const AppStartup::LaunchSeedParseResult seedParseResult =
        AppStartup::parseLaunchSeed(argc, argv);
    if (!seedParseResult.valid) {
        std::cerr << "Invalid seed argument: " << seedParseResult.invalidArgument << std::endl;
        return -1;
    }

    std::optional<Game> sim;
    if (seedParseResult.requestedSeed.has_value()) {
        sim.emplace(*seedParseResult.requestedSeed);
    } else {
        sim.emplace();
    }

    const Random::SeedSet& seeds = sim->getRngSeeds();
    std::cout << "rng seeds base=" << seeds.baseSeed << " sim=" << seeds.simSeed
              << " fx=" << seeds.fxSeed << std::endl;

    Platform platform;

    if (!platform.initialize())
        return -1;

    platform.run(*sim);

    return 0;
}
