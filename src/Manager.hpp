#pragma once
#include <array>
#include <optional>
#include <random>
#include <vector>
#include "Card.hpp"
#include "Deck.hpp"

// État du jeu et règles pour 2 joueurs. Aucune dépendance à SFML.
class Manager {
public:
    static constexpr int kTargetScore = 67;  // points pour gagner une manche
    static constexpr int kMatchTarget = 7;   // points de partie pour gagner la partie
    static constexpr int kHandSize = 5;

    enum class Phase {
        Lead,      // le meneur doit poser une carte
        Follow,    // l'adversaire doit répondre
        TrickDone, // pli résolu, en attente de next()
        RoundOver, // manche terminée, en attente de nextRound()
        GameOver   // un joueur a atteint kMatchTarget points de partie
    };

    Manager();

    // Nouvelle partie. startPlayer : le joueur le plus jeune, qui ouvre la première manche.
    void newGame(int startPlayer = 0);

    // Après une manche terminée : nouvelle manche, l'ouverture alterne entre les joueurs.
    void nextRound();

    // Le joueur courant pose la carte d'indice handIndex. Faux si coup invalide.
    bool play(int handIndex);

    // Le joueur courant peut-il poser cette carte ? Quand la pioche est vide, celui qui
    // répond au meneur doit jouer une carte qui gagne le pli s'il en a une.
    bool isLegal(int handIndex) const;

    // Après un pli résolu : le gagnant pioche en premier, puis nouveau pli.
    void next();

    Phase phase() const { return phase_; }
    int currentPlayer() const; // valide en phase Lead/Follow
    int leader() const { return leader_; }
    int trickWinner() const { return winner_; } // valide en TrickDone/GameOver
    const std::vector<Card>& hand(int player) const { return hands_[player]; }
    int score(int player) const { return scores_[player]; }
    const std::optional<Card>& played(int player) const { return played_[player]; }
    std::size_t deckSize() const { return deck_.size(); }

    // Carte révélée après la distribution, envoyée en bas du paquet face visible.
    // Sa saison est la saison forte de la partie.
    const Card& revealedCard() const { return revealed_; }
    Season strongSeason() const { return revealed_.season; }

    // Points de partie (0..kMatchTarget), cumulés sur les manches.
    int matchScore(int player) const { return matchScores_[player]; }
    // Vainqueur de la dernière manche (-1 si personne n'a atteint 67) et points de partie
    // qu'il a obtenus. Valides en RoundOver/GameOver.
    int roundWinner() const { return roundWinner_; }
    int roundPoints() const { return roundPoints_; }
    // Valide en GameOver.
    int gameWinner() const;

private:
    void draw(int player);
    bool followerWins(const Card& lead, const Card& follow) const;
    void startRound(int startPlayer);
    void endRound();

    std::mt19937 rng_;
    Deck deck_;
    std::array<std::vector<Card>, 2> hands_;
    std::array<int, 2> scores_{};      // points de la manche en cours
    std::array<int, 2> matchScores_{}; // points de partie
    std::array<std::optional<Card>, 2> played_;
    Card revealed_{Season::Spring, Rank::Prince};
    int leader_ = 0;
    int firstLeader_ = 0; // qui a ouvert la manche en cours
    int winner_ = -1;
    int roundWinner_ = -1;
    int roundPoints_ = 0;
    Phase phase_ = Phase::Lead;
};
