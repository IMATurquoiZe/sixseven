#include "Deck.hpp"
#include <algorithm>
#include <stdexcept>

Deck::Deck() {
    for (int s = 0; s < 4; ++s)
        for (int r = 0; r < 6; ++r)
            cards_.push_back({static_cast<Season>(s), static_cast<Rank>(r)});
}

void Deck::shuffle(std::mt19937& rng) {
    std::shuffle(cards_.begin(), cards_.end(), rng);
}

Card Deck::moveTopToBottom() {
    if (cards_.empty()) throw std::runtime_error("Deck vide");
    Card c = cards_.back();
    cards_.pop_back();
    cards_.insert(cards_.begin(), c);
    return c;
}

Card Deck::draw() {
    if (cards_.empty()) throw std::runtime_error("Deck vide");
    Card c = cards_.back();
    cards_.pop_back();
    return c;
}
