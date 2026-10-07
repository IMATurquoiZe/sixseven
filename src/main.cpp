#include <SFML/Graphics.hpp>
#include <string>
#include "AI.hpp"
#include "Manager.hpp"

namespace {

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

// Indice de la carte sous la souris dans la main du joueur, ou -1.
int hitHand(const Manager& game, int player, float mx, float my) {
    float y = player == 0 ? kBottomY : kTopY;
    if (my < y || my > y + kH) return -1;
    float x0 = handStartX(game.hand(player).size());
    for (std::size_t i = 0; i < game.hand(player).size(); ++i) {
        float x = x0 + i * (kW + kGap);
        if (mx >= x && mx <= x + kW) return static_cast<int>(i);
    }
    return -1;
}

constexpr int kHuman = 0; // joueur du bas ; l'IA est le joueur du haut
const sf::FloatRect kAnnounceBtn = {330, kWinH - 46, 180, 34};

void drawCardBack(sf::RenderWindow& win, float x, float y) {
    sf::RectangleShape rect({kW, kH});
    rect.setPosition(x, y);
    rect.setFillColor(sf::Color(40, 50, 120));
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);
    win.draw(rect);
}

void drawButton(sf::RenderWindow& win, const sf::Font& font, const sf::FloatRect& r, bool enabled) {
    sf::RectangleShape rect({r.width, r.height});
    rect.setPosition(r.left, r.top);
    rect.setFillColor(enabled ? sf::Color(200, 60, 60) : sf::Color(90, 90, 90));
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);
    win.draw(rect);
    drawText(win, font, "Annoncer 67", 18, r.left + 22, r.top + 5, sf::Color::White);
}

std::string playerName(int p) { return p == kHuman ? "Le joueur" : "L'IA"; }

} // namespace

int main() {
    sf::RenderWindow window(sf::VideoMode(kWinW, kWinH), "sixseven");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("assets/font.ttf")) return 1;

    Manager game;
    game.newGame();
    AI ai(1 - kHuman);
    sf::Clock aiClock; // délai de réflexion de l'IA

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            bool advance = (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Space);
            if (advance) {
                if (game.phase() == Manager::Phase::TrickDone) game.next();
                else if (game.phase() == Manager::Phase::RoundOver) game.nextRound();
                else if (game.phase() == Manager::Phase::GameOver) game.newGame();
            }

            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f pos(e.mouseButton.x, e.mouseButton.y);
                if (kAnnounceBtn.contains(pos)) game.announce(kHuman);
            }

            if (e.type == sf::Event::MouseButtonPressed &&
                e.mouseButton.button == sf::Mouse::Left &&
                (game.phase() == Manager::Phase::Lead || game.phase() == Manager::Phase::Follow) &&
                game.currentPlayer() == kHuman) {
                int idx = hitHand(game, kHuman, e.mouseButton.x, e.mouseButton.y);
                if (idx >= 0) game.play(idx);
            }
        }

        // Tour de l'IA : annonce dès qu'elle a 67, sinon joue après une courte pause.
        if (ai.shouldAnnounce(game)) {
            game.announce(ai.player());
        } else if ((game.phase() == Manager::Phase::Lead || game.phase() == Manager::Phase::Follow) &&
                   game.currentPlayer() == ai.player()) {
            if (aiClock.getElapsedTime().asSeconds() > 0.8f) {
                game.play(ai.chooseCard(game));
                aiClock.restart();
            }
        } else {
            aiClock.restart();
        }

        window.clear(sf::Color(30, 100, 60));
        const sf::Color white = sf::Color::White;
        const auto phase = game.phase();
        const bool playing = phase == Manager::Phase::Lead || phase == Manager::Phase::Follow;

        // Scores et pioche
        auto scoreLine = [&](int p) {
            return playerName(p) + " — Partie : " + std::to_string(game.matchScore(p)) + " / " +
                   std::to_string(Manager::kMatchTarget);
        };
        drawText(window, font, scoreLine(ai.player()), 20, 20, 10, white);
        drawText(window, font, scoreLine(kHuman), 20, 20, kWinH - 40, white);
        drawText(window, font, "Pioche : " + std::to_string(game.deckSize()), 20, kWinW - 160, 10, white);

        drawButton(window, font, kAnnounceBtn, game.canAnnounce());

        // Mains
        for (int p = 0; p < 2; ++p) {
            float y = p == kHuman ? kBottomY : kTopY;
            float x = handStartX(game.hand(p).size());
            bool active = playing && game.currentPlayer() == p;
            for (std::size_t i = 0; i < game.hand(p).size(); ++i) {
                if (p == kHuman)
                    drawCard(window, font, game.hand(p)[i], x, y, active && game.isLegal(static_cast<int>(i)));
                else
                    drawCardBack(window, x, y); // main de l'IA cachée
                x += kW + kGap;
            }
        }

        // Cartes posées au centre : joueur 1 à gauche, joueur 2 à droite
        for (int p = 0; p < 2; ++p) {
            if (!game.played(p)) continue;
            float x = kWinW / 2.f + (p == kHuman ? -kW - 20 : 20);
            drawCard(window, font, *game.played(p), x, kMidY, false);
        }

        // Carte révélée = saison forte
        {
            const Card& r = game.revealedCard();
            drawText(window, font, "Saison forte", 16, 40, kMidY - 24, white);
            drawText(window, font, r.seasonName(), 16, 40, kMidY - 4, seasonColor(r.season));
            drawCard(window, font, r, 40, kMidY + 20, false);
        }

        // Message d'état
        std::string msg;
        if (playing) {
            msg = game.currentPlayer() == kHuman ? "À toi de jouer" : "L'IA réfléchit...";
        } else if (phase == Manager::Phase::TrickDone) {
            msg = playerName(game.trickWinner()) + " remporte le pli. [Espace] pour continuer";
        } else {
            int rw = game.roundWinner();
            if (game.announcer() >= 0)
                msg = playerName(game.announcer()) + (game.announceCorrect() ? " : annonce juste. "
                                                                              : " : fausse annonce. ");
            if (rw < 0) msg += "Manche sans vainqueur";
            else msg += playerName(rw) + " gagne " + std::to_string(game.roundPoints()) + " pt(s)";
            // Les points de manche, cachés pendant le jeu, sont révélés.
            drawText(window, font,
                     "Points de manche — " + playerName(kHuman) + " : " + std::to_string(game.score(kHuman)) +
                         "   " + playerName(ai.player()) + " : " + std::to_string(game.score(ai.player())),
                     20, 200, kMidY + 60, white, kWinW - 220);
            if (phase == Manager::Phase::RoundOver)
                msg += ". [Espace] manche suivante";
            else
                msg += ". Partie gagnée ! [Espace] rejouer";
        }
        drawText(window, font, msg, 20, 200, kMidY - 32, sf::Color::Yellow, kWinW - 220);

        window.display();
    }
}
