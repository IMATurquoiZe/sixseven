#include "Manager.hpp"

Manager::Manager() : rng_(std::random_device{}()) {}

void Manager::newGame(int startPlayer) {
    matchScores_ = {0, 0};
    startRound(startPlayer);
}

void Manager::nextRound() {
    if (phase_ != Phase::RoundOver) return;
    startRound(1 - firstLeader_);
}

void Manager::startRound(int startPlayer) {
    firstLeader_ = startPlayer;
    roundWinner_ = -1;
    roundPoints_ = 0;
    deck_ = Deck();
    deck_.shuffle(rng_);
    for (auto& h : hands_) h.clear();
    scores_ = {0, 0};
    played_ = {};
    leader_ = startPlayer;
    winner_ = -1;
    phase_ = Phase::Lead;
    for (int i = 0; i < kHandSize; ++i) {
        draw(leader_);
        draw(1 - leader_);
    }
    revealed_ = deck_.moveTopToBottom();
}

int Manager::currentPlayer() const {
    return phase_ == Phase::Follow ? 1 - leader_ : leader_;
}

// Force des saisons dans le pli : la saison forte (2) bat la saison de la carte
// d'ouverture (1), qui bat toutes les autres (0). À force égale, le niveau le plus
// haut gagne ; à égalité de niveau, le meneur garde la main.
bool Manager::followerWins(const Card& lead, const Card& follow) const {
    auto power = [&](const Card& c) {
        if (c.season == strongSeason()) return 2;
        if (c.season == lead.season) return 1;
        return 0;
    };
    const int leadPower = power(lead), followPower = power(follow);
    if (followPower != leadPower) return followPower > leadPower;
    return follow.level() > lead.level();
}

bool Manager::isLegal(int handIndex) const {
    if (phase_ != Phase::Lead && phase_ != Phase::Follow) return false;
    const auto& hand = hands_[currentPlayer()];
    if (handIndex < 0 || handIndex >= static_cast<int>(hand.size())) return false;
    if (phase_ == Phase::Lead || !deck_.empty()) return true;

    // Pioche vide : obligation de gagner le pli si possible.
    const Card& lead = *played_[leader_];
    bool canWin = false;
    for (const Card& c : hand) canWin = canWin || followerWins(lead, c);
    return !canWin || followerWins(lead, hand[handIndex]);
}

bool Manager::play(int handIndex) {
    if (phase_ != Phase::Lead && phase_ != Phase::Follow) return false;
    int p = currentPlayer();
    auto& hand = hands_[p];
    if (!isLegal(handIndex)) return false;

    played_[p] = hand[handIndex];
    hand.erase(hand.begin() + handIndex);

    if (phase_ == Phase::Lead) {
        phase_ = Phase::Follow;
        return true;
    }

    const Card& lead = *played_[leader_];
    const Card& follow = *played_[p];
    winner_ = followerWins(lead, follow) ? p : leader_;
    scores_[winner_] += lead.points() + follow.points();
    phase_ = Phase::TrickDone;
    return true;
}

void Manager::next() {
    if (phase_ != Phase::TrickDone) return;

    draw(winner_);
    draw(1 - winner_);
    played_ = {};
    leader_ = winner_;

    bool reached = scores_[0] >= kTargetScore || scores_[1] >= kTargetScore;
    bool noCards = hands_[0].empty() && hands_[1].empty();
    if (reached || noCards) endRound();
    else phase_ = Phase::Lead;
}

// Manche gagnée à 67 points. Points de partie selon le score du perdant :
// 0 -> 3, 1..32 -> 2, 33 et plus -> 1. Sans 67 (plus de cartes), personne ne marque.
void Manager::endRound() {
    roundWinner_ = -1;
    roundPoints_ = 0;
    for (int p = 0; p < 2; ++p) {
        if (scores_[p] < kTargetScore) continue;
        int loser = scores_[1 - p];
        roundWinner_ = p;
        roundPoints_ = loser == 0 ? 3 : (loser < 33 ? 2 : 1);
        matchScores_[p] += roundPoints_;
    }
    phase_ = (matchScores_[0] >= kMatchTarget || matchScores_[1] >= kMatchTarget)
                 ? Phase::GameOver : Phase::RoundOver;
}

int Manager::gameWinner() const {
    if (matchScores_[0] == matchScores_[1]) return -1;
    return matchScores_[0] > matchScores_[1] ? 0 : 1;
}

void Manager::draw(int player) {
    if (!deck_.empty()) hands_[player].push_back(deck_.draw());
}
