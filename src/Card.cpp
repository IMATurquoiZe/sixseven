#include "Card.hpp"

int Card::level() const { return static_cast<int>(rank) + 1; }

int Card::points() const {
    switch (rank) {
        case Rank::Prince:   return 0;
        case Rank::Treasure: return 10;
        case Rank::Hero:     return 2;
        case Rank::Queen:    return 3;
        case Rank::King:     return 4;
        case Rank::Divinity: return 11;
    }
    return 0;
}

std::string Card::name() const {
    switch (rank) {
        case Rank::Prince:   return "Prince";
        case Rank::Treasure: return "Trésor";
        case Rank::Hero:     return "Héros";
        case Rank::Queen:    return "Reine";
        case Rank::King:     return "Roi";
        case Rank::Divinity: return "Divinité";
    }
    return "";
}

std::string Card::seasonName() const {
    switch (season) {
        case Season::Spring: return "Printemps";
        case Season::Summer: return "Été";
        case Season::Autumn: return "Automne";
        case Season::Winter: return "Hiver";
    }
    return "";
}
