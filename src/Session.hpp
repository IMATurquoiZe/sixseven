#pragma once
#include <SFML/Network.hpp>
#include <memory>
#include <string>
#include "AI.hpp"
#include "Manager.hpp"
#include "View.hpp"

constexpr unsigned short kDefaultPort = 53000;

enum class ActionType : sf::Uint8 { Play, Announce, Advance };

// Intention d'un joueur ; la partie décide si elle est valide.
struct Action {
    ActionType type = ActionType::Advance;
    int index = 0; // Play : indice de la carte dans la main
};

// Une partie vue par le joueur local, quelle que soit sa nature (solo, hôte, client).
class Session {
public:
    virtual ~Session() = default;

    virtual void send(const Action& action) = 0; // action du joueur local
    virtual void update() = 0;                   // à appeler à chaque image
    virtual std::string opponentName() const = 0;
    virtual bool connected() const { return true; }

    const GameView& view() const { return view_; }

protected:
    GameView view_;
};

// Partie hébergée sur cette machine : le joueur local est le joueur 0. L'adversaire est
// l'IA (sans socket) ou un client distant ; l'hôte est seul à détenir le Manager.
class LocalSession : public Session {
public:
    explicit LocalSession(std::unique_ptr<sf::TcpSocket> remote = nullptr);

    void send(const Action& action) override { apply(0, action); }
    void update() override;
    std::string opponentName() const override { return remote_ ? "L'adversaire" : "L'IA"; }
    bool connected() const override { return connected_; }

private:
    void apply(int player, const Action& action);
    void pollRemote();
    void pushView();
    GameView viewFor(int player) const;

    Manager game_;
    AI ai_{1};
    sf::Clock aiClock_; // délai de réflexion de l'IA
    std::unique_ptr<sf::TcpSocket> remote_;
    bool connected_ = true;
    std::array<bool, 2> ready_{}; // a validé la fin du pli / de la manche
    bool dirty_ = true; // l'état a changé depuis le dernier envoi au client
};

// Client connecté à un hôte : ne contient aucune règle, seulement la dernière vue reçue.
class ClientSession : public Session {
public:
    explicit ClientSession(std::unique_ptr<sf::TcpSocket> host) : host_(std::move(host)) {}

    void send(const Action& action) override;
    void update() override;
    std::string opponentName() const override { return "L'adversaire"; }
    bool connected() const override { return connected_; }

private:
    std::unique_ptr<sf::TcpSocket> host_;
    bool connected_ = true;
};
