#include "View.hpp"

namespace {

constexpr std::size_t kMaxHand = 16;

void writeCard(sf::Packet& p, const Card& c) {
    p << static_cast<sf::Uint8>(c.season) << static_cast<sf::Uint8>(c.rank);
}

bool readCard(sf::Packet& p, Card& c) {
    sf::Uint8 s = 0, r = 0;
    p >> s >> r;
    if (s > static_cast<sf::Uint8>(Season::Winter) || r > static_cast<sf::Uint8>(Rank::Divinity)) return false;
    c = Card{static_cast<Season>(s), static_cast<Rank>(r)};
    return true;
}

} // namespace

GameView makeView(const Manager& game, int me) {
    auto rel = [&](int p) { return p < 0 ? -1 : (p == me ? 0 : 1); };
    const int opp = 1 - me;

    GameView v;
    v.phase = game.phase();
    v.current = v.playing() ? rel(game.currentPlayer()) : -1;
    v.trickWinner = v.phase == Manager::Phase::TrickDone ? rel(game.trickWinner()) : -1;
    v.hand = game.hand(me);
    v.legal.assign(v.hand.size(), 0);
    if (v.current == 0)
        for (std::size_t i = 0; i < v.hand.size(); ++i)
            v.legal[i] = game.isLegal(static_cast<int>(i));
    v.opponentHandSize = static_cast<int>(game.hand(opp).size());
    v.played = {game.played(me), game.played(opp)};
    v.deckSize = static_cast<int>(game.deckSize());
    v.revealed = game.revealedCard();
    v.matchScore = {game.matchScore(me), game.matchScore(opp)};
    v.canAnnounce = v.playing() && game.canAnnounce();

    const bool over = v.phase == Manager::Phase::RoundOver || v.phase == Manager::Phase::GameOver;
    if (over) {
        v.roundWinner = rel(game.roundWinner());
        v.roundPoints = game.roundPoints();
        v.announcer = rel(game.announcer());
        v.announceCorrect = game.announceCorrect();
        v.score = {game.score(me), game.score(opp)};
    }
    return v;
}

void writeView(sf::Packet& p, const GameView& v) {
    p << static_cast<sf::Uint8>(v.phase) << static_cast<sf::Int8>(v.current)
      << static_cast<sf::Int8>(v.trickWinner);
    p << static_cast<sf::Uint8>(v.hand.size());
    for (std::size_t i = 0; i < v.hand.size(); ++i) {
        writeCard(p, v.hand[i]);
        p << static_cast<sf::Uint8>(v.legal[i]);
    }
    p << static_cast<sf::Uint8>(v.opponentHandSize);
    for (const auto& c : v.played) {
        p << static_cast<sf::Uint8>(c.has_value());
        if (c) writeCard(p, *c);
    }
    p << static_cast<sf::Uint8>(v.deckSize);
    writeCard(p, v.revealed);
    p << static_cast<sf::Uint8>(v.matchScore[0]) << static_cast<sf::Uint8>(v.matchScore[1]);
    p << static_cast<sf::Int8>(v.roundWinner) << static_cast<sf::Uint8>(v.roundPoints)
      << static_cast<sf::Int8>(v.announcer) << static_cast<sf::Uint8>(v.announceCorrect)
      << static_cast<sf::Int16>(v.score[0]) << static_cast<sf::Int16>(v.score[1])
      << static_cast<sf::Uint8>(v.canAnnounce)
      << static_cast<sf::Uint8>(v.iAmReady) << static_cast<sf::Uint8>(v.opponentReady);
}

bool readView(sf::Packet& p, GameView& v) {
    GameView out;
    sf::Uint8 u = 0;
    sf::Int8 i8 = 0;
    sf::Int16 i16 = 0;

    p >> u;
    if (u > static_cast<sf::Uint8>(Manager::Phase::GameOver)) return false;
    out.phase = static_cast<Manager::Phase>(u);
    p >> i8; out.current = i8;
    p >> i8; out.trickWinner = i8;

    p >> u;
    if (u > kMaxHand) return false;
    out.hand.resize(u, Card{Season::Spring, Rank::Prince});
    out.legal.resize(u);
    for (std::size_t i = 0; i < out.hand.size(); ++i) {
        if (!readCard(p, out.hand[i])) return false;
        p >> out.legal[i];
    }
    p >> u; out.opponentHandSize = u;
    for (auto& c : out.played) {
        p >> u;
        if (u) {
            Card card{Season::Spring, Rank::Prince};
            if (!readCard(p, card)) return false;
            c = card;
        }
    }
    p >> u; out.deckSize = u;
    if (!readCard(p, out.revealed)) return false;
    p >> u; out.matchScore[0] = u;
    p >> u; out.matchScore[1] = u;
    p >> i8; out.roundWinner = i8;
    p >> u; out.roundPoints = u;
    p >> i8; out.announcer = i8;
    p >> u; out.announceCorrect = u != 0;
    p >> i16; out.score[0] = i16;
    p >> i16; out.score[1] = i16;
    p >> u; out.canAnnounce = u != 0;
    p >> u; out.iAmReady = u != 0;
    p >> u; out.opponentReady = u != 0;

    if (!p) return false;
    v = std::move(out);
    return true;
}
