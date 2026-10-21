#pragma once
#include <SFML/Network/Packet.hpp>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>
#include "Manager.hpp"

// Ce qu'un joueur a le droit de voir de la partie. C'est la seule chose qui circule sur
// le réseau : ni la main adverse, ni les points de manche pendant le jeu.
// Les indices de joueur sont relatifs : 0 = moi, 1 = l'adversaire (-1 = personne).
struct GameView {
    Manager::Phase phase = Manager::Phase::Lead;
    int current = -1;      // joueur qui doit jouer (Lead/Follow), sinon -1
    int trickWinner = -1;  // valide en TrickDone
    std::vector<Card> hand;
    std::vector<std::uint8_t> legal; // legal[i] != 0 : je peux poser hand[i] maintenant
    int opponentHandSize = 0;
    std::array<std::optional<Card>, 2> played;
    int deckSize = 0;
    Card revealed{Season::Spring, Rank::Prince};
    std::array<int, 2> matchScore{};
    // Valides en RoundOver/GameOver.
    int roundWinner = -1;
    int roundPoints = 0;
    int announcer = -1;
    bool announceCorrect = false;
    std::array<int, 2> score{}; // points de manche, révélés seulement en fin de manche
    bool canAnnounce = false; // faux pendant le gel entre deux plis
    // Entre deux plis/manches, la partie attend que les deux joueurs aient validé.
    bool iAmReady = false;
    bool opponentReady = false;

    bool playing() const { return phase == Manager::Phase::Lead || phase == Manager::Phase::Follow; }
};

// Vue de la partie du point de vue du joueur `me` (0 ou 1).
GameView makeView(const Manager& game, int me);

void writeView(sf::Packet& packet, const GameView& view);
bool readView(sf::Packet& packet, GameView& view); // faux si le paquet est invalide
