#include "Session.hpp"

namespace {

// Les paquets sont minuscules : on envoie en mode bloquant pour éviter les envois partiels,
// puis on revient en non bloquant pour ne jamais figer la fenêtre en réception.
bool sendPacket(sf::TcpSocket& socket, sf::Packet& packet) {
    socket.setBlocking(true);
    const bool ok = socket.send(packet) == sf::Socket::Done;
    socket.setBlocking(false);
    return ok;
}

bool isPlaying(const Manager& game) {
    return game.phase() == Manager::Phase::Lead || game.phase() == Manager::Phase::Follow;
}

} // namespace

// --- LocalSession -----------------------------------------------------------------------

LocalSession::LocalSession(std::unique_ptr<sf::TcpSocket> remote) : remote_(std::move(remote)) {
    if (remote_) remote_->setBlocking(false);
    game_.newGame();
}

void LocalSession::apply(int player, const Action& action) {
    switch (action.type) {
        case ActionType::Play:
            if (isPlaying(game_) && game_.currentPlayer() == player) dirty_ |= game_.play(action.index);
            break;
        case ActionType::Announce:
            // Pas d'annonce pendant le gel entre deux plis.
            if (isPlaying(game_)) dirty_ |= game_.announce(player);
            break;
        case ActionType::Advance:
            if (isPlaying(game_)) break;
            // Les deux joueurs doivent valider ; l'IA valide toujours tout de suite.
            ready_[player] = true;
            if (!remote_) ready_[1] = true;
            dirty_ = true;
            if (ready_[0] && ready_[1]) {
                ready_ = {};
                switch (game_.phase()) {
                    case Manager::Phase::TrickDone: game_.next(); break;
                    case Manager::Phase::RoundOver: game_.nextRound(); break;
                    default: game_.newGame(); break;
                }
            }
            break;
    }
}

void LocalSession::pollRemote() {
    sf::Packet packet;
    sf::Socket::Status status;
    while ((status = remote_->receive(packet)) == sf::Socket::Done) {
        sf::Uint8 type = 0, index = 0;
        packet >> type;
        if (type > static_cast<sf::Uint8>(ActionType::Advance)) continue;
        if (static_cast<ActionType>(type) == ActionType::Play) packet >> index;
        if (!packet) continue;
        apply(1, Action{static_cast<ActionType>(type), index});
        packet.clear();
    }
    if (status == sf::Socket::Disconnected || status == sf::Socket::Error) connected_ = false;
}

GameView LocalSession::viewFor(int player) const {
    GameView v = makeView(game_, player);
    v.iAmReady = ready_[player];
    v.opponentReady = ready_[1 - player];
    return v;
}

void LocalSession::pushView() {
    sf::Packet packet;
    writeView(packet, viewFor(1));
    if (!sendPacket(*remote_, packet)) connected_ = false;
}

void LocalSession::update() {
    if (remote_) {
        if (connected_) pollRemote();
    } else {
        // Tour de l'IA : annonce dès qu'elle a 67, sinon joue après une courte pause.
        if (ai_.shouldAnnounce(game_)) {
            apply(ai_.player(), Action{ActionType::Announce});
        } else if (isPlaying(game_) && game_.currentPlayer() == ai_.player()) {
            if (aiClock_.getElapsedTime().asSeconds() > 0.8f) {
                apply(ai_.player(), Action{ActionType::Play, ai_.chooseCard(game_)});
                aiClock_.restart();
            }
        } else {
            aiClock_.restart();
        }
    }

    if (remote_ && connected_ && dirty_) pushView();
    dirty_ = false;
    view_ = viewFor(0);
}

// --- ClientSession ----------------------------------------------------------------------

void ClientSession::send(const Action& action) {
    if (!connected_) return;
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(action.type);
    if (action.type == ActionType::Play) packet << static_cast<sf::Uint8>(action.index);
    if (!sendPacket(*host_, packet)) connected_ = false;
}

void ClientSession::update() {
    if (!connected_) return;
    sf::Packet packet;
    sf::Socket::Status status;
    while ((status = host_->receive(packet)) == sf::Socket::Done) {
        readView(packet, view_);
        packet.clear();
    }
    if (status == sf::Socket::Disconnected || status == sf::Socket::Error) connected_ = false;
}
