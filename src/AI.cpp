#include "AI.hpp"
#include <tuple>
#include <vector>

bool AI::shouldAnnounce(const Manager& game) const {
    return game.canAnnounce() && game.score(player_) >= Manager::kTargetScore;
}

int AI::chooseCard(const Manager& game) const {
    const auto& hand = game.hand(player_);
    std::vector<int> legal;
    for (int i = 0; i < static_cast<int>(hand.size()); ++i)
        if (game.isLegal(i)) legal.push_back(i);
    if (legal.empty()) return -1;

    // Carte la moins coûteuse : le moins de points, puis le niveau le plus bas.
    auto cheapest = [&](const std::vector<int>& idx) {
        int best = idx.front();
        for (int i : idx) {
            auto key = [&](int k) { return std::make_tuple(hand[k].points(), hand[k].level()); };
            if (key(i) < key(best)) best = i;
        }
        return best;
    };

    if (game.phase() == Manager::Phase::Follow) {
        const Card& lead = *game.played(game.leader());
        std::vector<int> winning;
        for (int i : legal)
            if (game.followerWins(lead, hand[i])) winning.push_back(i);
        // Gagner au plus juste, sinon sacrifier la carte la moins précieuse.
        return cheapest(winning.empty() ? legal : winning);
    }

    // Ouverture : la carte la plus forte (saison forte d'abord, puis niveau).
    int best = legal.front();
    for (int i : legal) {
        auto key = [&](int k) {
            return std::make_tuple(hand[k].season == game.strongSeason(), hand[k].level(),
                                   -hand[k].points());
        };
        if (key(i) > key(best)) best = i;
    }
    return best;
}
