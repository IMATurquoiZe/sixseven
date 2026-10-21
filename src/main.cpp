#include <SFML/Graphics.hpp>
#include <filesystem>
#include <string>
#include <memory>
#include "Session.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

// Dossier contenant l'exécutable, pour retrouver assets/ quel que soit le dossier courant.
std::filesystem::path exeDir() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) return std::filesystem::path(buf).parent_path();
#else
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) { buf[n] = '\0'; return std::filesystem::path(buf).parent_path(); }
#endif
    return std::filesystem::current_path();
}

constexpr float kW = 120, kH = 160, kGap = 10;
constexpr unsigned kWinW = 1000, kWinH = 640;
constexpr float kTopY = 50, kMidY = 240, kBottomY = 430;

sf::Color seasonColor(Season s) {
    switch (s) {
        case Season::Spring: return sf::Color(40, 150, 60);
        case Season::Summer: return sf::Color(210, 150, 0);
        case Season::Autumn: return sf::Color(170, 70, 20);
        default:             return sf::Color(50, 100, 200);
    }
}

float handStartX(std::size_t n) {
    return (kWinW - (n * (kW + kGap) - kGap)) / 2;
}

// maxWidth > 0 : le texte est réduit s'il dépasse cette largeur.
void drawText(sf::RenderWindow& win, const sf::Font& font, const std::string& str,
              unsigned size, float x, float y, sf::Color color, float maxWidth = 0) {
    sf::Text t(sf::String::fromUtf8(str.begin(), str.end()), font, size);
    t.setFillColor(color);
    t.setPosition(x, y);
    float w = t.getLocalBounds().width;
    if (maxWidth > 0 && w > maxWidth) t.setScale(maxWidth / w, maxWidth / w);
    win.draw(t);
}

void drawCard(sf::RenderWindow& win, const sf::Font& font, const Card& c,
              float x, float y, bool highlight) {
    sf::RectangleShape rect({kW, kH});
    rect.setPosition(x, y);
    rect.setFillColor(sf::Color::White);
    rect.setOutlineColor(highlight ? sf::Color::Yellow : sf::Color::Black);
    rect.setOutlineThickness(highlight ? 4 : 2);
    win.draw(rect);

    sf::Color col = seasonColor(c.season);
    drawText(win, font, c.name(), 20, x + 8, y + 8, col);
    drawText(win, font, c.seasonName(), 16, x + 8, y + 40, col);
    drawText(win, font, "Niv. " + std::to_string(c.level()), 16, x + 8, y + 100, col);
}

// Indice de la carte sous la souris dans la main du joueur local (en bas), ou -1.
int hitHand(const GameView& v, float mx, float my) {
    if (my < kBottomY || my > kBottomY + kH) return -1;
    float x0 = handStartX(v.hand.size());
    for (std::size_t i = 0; i < v.hand.size(); ++i) {
        float x = x0 + i * (kW + kGap);
        if (mx >= x && mx <= x + kW) return static_cast<int>(i);
    }
    return -1;
}

const sf::FloatRect kAnnounceBtn = {330, kWinH - 46, 180, 34};

void drawCardBack(sf::RenderWindow& win, float x, float y) {
    sf::RectangleShape rect({kW, kH});
    rect.setPosition(x, y);
    rect.setFillColor(sf::Color(40, 50, 120));
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);
    win.draw(rect);
}

void drawButton(sf::RenderWindow& win, const sf::Font& font, const sf::FloatRect& r,
                const std::string& label, bool enabled, sf::Color color = sf::Color(200, 60, 60)) {
    sf::RectangleShape rect({r.width, r.height});
    rect.setPosition(r.left, r.top);
    rect.setFillColor(enabled ? color : sf::Color(90, 90, 90));
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);
    win.draw(rect);
    sf::Text t(sf::String::fromUtf8(label.begin(), label.end()), font, 18);
    auto b = t.getLocalBounds();
    t.setPosition(r.left + (r.width - b.width) / 2 - b.left, r.top + (r.height - b.height) / 2 - b.top);
    t.setFillColor(sf::Color::White);
    win.draw(t);
}

void drawGame(sf::RenderWindow& window, const sf::Font& font, const Session& session) {
    const GameView& v = session.view();
    const std::string me = "Le joueur", opp = session.opponentName();
    auto name = [&](int p) { return p == 0 ? me : opp; };
    const sf::Color white = sf::Color::White;
    const auto phase = v.phase;

    // Scores et pioche
    auto scoreLine = [&](int p) {
        return name(p) + " — Partie : " + std::to_string(v.matchScore[p]) + " / " +
               std::to_string(Manager::kMatchTarget);
    };
    drawText(window, font, scoreLine(1), 20, 20, 10, white);
    drawText(window, font, scoreLine(0), 20, 20, kWinH - 40, white);
    drawText(window, font, "Pioche : " + std::to_string(v.deckSize), 20, kWinW - 160, 10, white);

    drawButton(window, font, kAnnounceBtn, "Annoncer 67", v.canAnnounce);

    // Mains : la mienne visible en bas, celle de l'adversaire cachée en haut
    {
        float x = handStartX(v.hand.size());
        for (std::size_t i = 0; i < v.hand.size(); ++i, x += kW + kGap)
            drawCard(window, font, v.hand[i], x, kBottomY, v.legal[i] != 0);
        x = handStartX(v.opponentHandSize);
        for (int i = 0; i < v.opponentHandSize; ++i, x += kW + kGap) drawCardBack(window, x, kTopY);
    }

    // Cartes posées au centre : moi à gauche, l'adversaire à droite
    for (int p = 0; p < 2; ++p) {
        if (!v.played[p]) continue;
        float x = kWinW / 2.f + (p == 0 ? -kW - 20 : 20);
        drawCard(window, font, *v.played[p], x, kMidY, false);
    }

    // Carte révélée = saison forte
    drawText(window, font, "Saison forte", 16, 40, kMidY - 24, white);
    drawText(window, font, v.revealed.seasonName(), 16, 40, kMidY - 4, seasonColor(v.revealed.season));
    drawCard(window, font, v.revealed, 40, kMidY + 20, false);

    // Message d'état
    std::string msg;
    const std::string continueHint = v.iAmReady ? "en attente de " + opp + "..."
                                                : (v.opponentReady ? opp + " est prêt. " : "") +
                                                      "[Clic] pour continuer";
    if (!session.connected()) {
        msg = "Connexion perdue. [Échap] pour revenir au menu";
    } else if (v.playing()) {
        msg = v.current == 0 ? "À toi de jouer" : opp + " réfléchit...";
    } else if (phase == Manager::Phase::TrickDone) {
        msg = name(v.trickWinner) + " remporte le pli. " + continueHint;
    } else {
        if (v.announcer >= 0)
            msg = name(v.announcer) + (v.announceCorrect ? " : annonce juste. " : " : fausse annonce. ");
        if (v.roundWinner < 0) msg += "Manche sans vainqueur";
        else msg += name(v.roundWinner) + " gagne " + std::to_string(v.roundPoints) + " pt(s)";
        // Les points de manche, cachés pendant le jeu, sont révélés.
        drawText(window, font,
                 "Points de manche — " + me + " : " + std::to_string(v.score[0]) + "   " + opp +
                     " : " + std::to_string(v.score[1]),
                 20, 200, kMidY + 60, white, kWinW - 220);
        msg += phase == Manager::Phase::RoundOver ? ". Manche suivante : " + continueHint
                                                  : ". Partie gagnée ! Rejouer : " + continueHint;
    }
    drawText(window, font, msg, 20, 200, kMidY - 32, sf::Color::Yellow, kWinW - 220);
}

enum class Screen { Menu, Hosting, Joining, Playing };

const sf::FloatRect kSoloBtn = {kWinW / 2.f - 150, 230, 300, 50};
const sf::FloatRect kHostBtn = {kWinW / 2.f - 150, 300, 300, 50};
const sf::FloatRect kJoinBtn = {kWinW / 2.f - 150, 370, 300, 50};

// "adresse" ou "adresse:port"
bool connectTo(const std::string& input, std::unique_ptr<Session>& session, std::string& error) {
    std::string host = input;
    unsigned port = kDefaultPort;
    if (auto colon = input.rfind(':'); colon != std::string::npos) {
        host = input.substr(0, colon);
        try { port = std::stoul(input.substr(colon + 1)); } catch (...) { port = 0; }
        if (port == 0 || port > 65535) { error = "Port invalide"; return false; }
    }
    sf::IpAddress ip(host);
    if (host.empty() || ip == sf::IpAddress::None) { error = "Adresse invalide"; return false; }
    auto socket = std::make_unique<sf::TcpSocket>();
    if (socket->connect(ip, static_cast<unsigned short>(port), sf::seconds(5)) != sf::Socket::Done) {
        error = "Connexion impossible à " + host + ":" + std::to_string(port);
        return false;
    }
    socket->setBlocking(false);
    session = std::make_unique<ClientSession>(std::move(socket));
    return true;
}

} // namespace

int main() {
    sf::RenderWindow window(sf::VideoMode(kWinW, kWinH), "sixseven");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile((exeDir() / "assets" / "font.ttf").string())) return 1;

    Screen screen = Screen::Menu;
    std::unique_ptr<Session> session;
    sf::TcpListener listener;
    std::string ipInput, error;

    auto backToMenu = [&] {
        listener.close();
        session.reset();
        screen = Screen::Menu;
    };

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape &&
                screen != Screen::Menu) {
                backToMenu();
                continue;
            }

            const bool click = e.type == sf::Event::MouseButtonPressed &&
                               e.mouseButton.button == sf::Mouse::Left;
            const sf::Vector2f pos = click ? sf::Vector2f(e.mouseButton.x, e.mouseButton.y) : sf::Vector2f();

            switch (screen) {
                case Screen::Menu:
                    if (!click) break;
                    error.clear();
                    if (kSoloBtn.contains(pos)) {
                        session = std::make_unique<LocalSession>();
                        screen = Screen::Playing;
                    } else if (kHostBtn.contains(pos)) {
                        listener.setBlocking(false);
                        if (listener.listen(kDefaultPort) == sf::Socket::Done) screen = Screen::Hosting;
                        else error = "Port " + std::to_string(kDefaultPort) + " indisponible";
                    } else if (kJoinBtn.contains(pos)) {
                        screen = Screen::Joining;
                    }
                    break;

                case Screen::Hosting:
                    break;

                case Screen::Joining:
                    if (e.type == sf::Event::TextEntered) {
                        if (e.text.unicode == '\b') { if (!ipInput.empty()) ipInput.pop_back(); }
                        else if (e.text.unicode > 32 && e.text.unicode < 127 && ipInput.size() < 64)
                            ipInput += static_cast<char>(e.text.unicode);
                    } else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter) {
                        error = "Connexion...";
                        window.clear(sf::Color(30, 100, 60));
                        drawText(window, font, error, 22, kWinW / 2.f - 70, 330, sf::Color::Yellow);
                        window.display();
                        error.clear();
                        if (connectTo(ipInput, session, error)) screen = Screen::Playing;
                    }
                    break;

                case Screen::Playing: {
                    const GameView& v = session->view();
                    if (!click) break;
                    if (kAnnounceBtn.contains(pos)) {
                        if (v.canAnnounce) session->send(Action{ActionType::Announce});
                    } else if (int idx = hitHand(v, pos.x, pos.y); idx >= 0) {
                        if (v.playing() && v.current == 0 && v.legal[idx])
                            session->send(Action{ActionType::Play, idx});
                    } else if (!v.playing() && !v.iAmReady) {
                        // Clic dans le vide : pli / manche suivante, ou nouvelle partie.
                        session->send(Action{ActionType::Advance});
                    }
                    break;
                }
            }
        }

        // Hôte : attente d'un adversaire
        if (screen == Screen::Hosting) {
            auto socket = std::make_unique<sf::TcpSocket>();
            if (listener.accept(*socket) == sf::Socket::Done) {
                listener.close();
                session = std::make_unique<LocalSession>(std::move(socket));
                screen = Screen::Playing;
            }
        }
        if (session) session->update();

        window.clear(sf::Color(30, 100, 60));
        const sf::Color white = sf::Color::White;
        switch (screen) {
            case Screen::Menu:
                drawText(window, font, "sixseven", 56, kWinW / 2.f - 130, 100, white);
                drawButton(window, font, kSoloBtn, "Solo (contre l'IA)", true, sf::Color(40, 100, 180));
                drawButton(window, font, kHostBtn, "Héberger une partie", true, sf::Color(40, 100, 180));
                drawButton(window, font, kJoinBtn, "Rejoindre une partie", true, sf::Color(40, 100, 180));
                drawText(window, font, error, 20, kWinW / 2.f - 150, 450, sf::Color::Yellow);
                break;
            case Screen::Hosting:
                drawText(window, font, "En attente d'un adversaire...", 30, kWinW / 2.f - 190, 220, white);
                drawText(window, font, "Adresse : " + sf::IpAddress::getLocalAddress().toString() + ":" +
                                           std::to_string(kDefaultPort),
                         22, kWinW / 2.f - 190, 290, sf::Color::Yellow);
                drawText(window, font, "[Échap] pour annuler", 18, kWinW / 2.f - 190, 340, white);
                break;
            case Screen::Joining:
                drawText(window, font, "Adresse de l'hôte (adresse[:port])", 26, kWinW / 2.f - 230, 200, white);
                drawText(window, font, ipInput + "_", 30, kWinW / 2.f - 230, 260, sf::Color::Yellow);
                drawText(window, font, error, 20, kWinW / 2.f - 230, 330, sf::Color(255, 160, 160));
                drawText(window, font, "[Entrée] pour se connecter, [Échap] pour annuler", 18,
                         kWinW / 2.f - 230, 380, white);
                break;
            case Screen::Playing:
                drawGame(window, font, *session);
                break;
        }
        window.display();
    }
}
