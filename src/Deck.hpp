#pragma once
#include <random>
#include <vector>
#include "Card.hpp"

class Deck {
public:
    Deck(); // 24 cartes : chaque rang dans chaque saison
    void shuffle(std::mt19937& rng);
    Card draw(); // pioche la carte du dessus
    Card moveTopToBottom(); // envoie la carte du dessus en bas du paquet et la renvoie
    bool empty() const { return cards_.empty(); }
    std::size_t size() const { return cards_.size(); }

private:
    std::vector<Card> cards_;
};
