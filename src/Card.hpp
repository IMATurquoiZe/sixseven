#pragma once
#include <string>

enum class Season { Spring, Summer, Autumn, Winter };

// Ordre croissant de hiérarchie : Prince (niveau 1) .. Divinity (niveau 6)
enum class Rank { Prince, Treasure, Hero, Queen, King, Divinity };

struct Card {
    Season season;
    Rank rank;

    int level() const;  // 1 (le plus bas) .. 6 (le plus haut)
    int points() const;
    std::string name() const;
    std::string seasonName() const;
};
