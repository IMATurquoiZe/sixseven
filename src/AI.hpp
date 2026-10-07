#pragma once
#include "Manager.hpp"

// Adversaire automatique. N'utilise que sa propre main, les cartes posées et la saison
// forte, plus les points de manche (connus de l'IA : elle ne se trompe jamais sur le 67).
class AI {
public:
    explicit AI(int player = 1) : player_(player) {}

    int player() const { return player_; }

    // L'IA annonce 67 dès qu'elle l'a atteint.
    bool shouldAnnounce(const Manager& game) const;

    // Indice de la carte à jouer dans sa main (toujours un coup légal).
    int chooseCard(const Manager& game) const;

private:
    int player_;
};
