#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <set>

class Game
{
public:
    explicit Game();
    void run();
private:
    enum
    {
        FPS = 5,
        MAP_WIDTH = 16,
        MAP_HEIGHT = 16,
        CELL_SIZE = 40,
        BORDER_SIZE = 5,
        SCREEN_WIDTH = MAP_WIDTH * CELL_SIZE + BORDER_SIZE,
        SCREEN_HEIGHT = MAP_HEIGHT * CELL_SIZE + BORDER_SIZE,
        OBSTACLE = 1000,
        UNVISITED = -1
    };

    enum class Connectivity { FOUR, EIGHT };

    const std::string TITLE_OF_MAIN_WINDOW { "FlowFields" };
    const sf::Time mFrameTime { sf::seconds(1.0f /FPS) };
    sf::RenderWindow mWindow;
    sf::Font mFont;

    struct Node
    {
        int x, y;
        int distance;
        bool obstacle;
        std::vector<Node*> neighbours;
        float vx, vy;
    };
    Node mMap[MAP_WIDTH][MAP_HEIGHT];
    Node *mStart;
    Node *mEnd;
    struct NodeCompare
    {
        bool operator()(Node* lhs, Node* rhs)
        {
            auto index = [](int x, int y){ return y * MAP_WIDTH + x; };
            return index(lhs->x, lhs->y) < index(rhs->x, rhs->y);
        }
    };
    std::set<Node*, NodeCompare> mOldWave;
    std::set<Node*, NodeCompare> mCurrentWave;
    std::vector<Node*> mShortestPath;
    bool mAlgorythmFinished = false;
    bool mIsPaused = false;
    bool mPathFound = false;
    float mMaxDist = 0;
    bool mPathRestored = false;

    void restart();
    void initMap();
    void resetMap();
    void createConnections(Connectivity connectivity);
    void restorePath();
    void calcGradients();
    bool isValidCoords(int x, int y) const;
    void inputPhase();
    void updatePhase(sf::Time frameTime);
    void renderPhase();
    void drawMap();
    void drawCurrentWave();
    void drawGradients();
    void drawPath();
    void drawLine(float x1, float y1, float x2, float y2, float thickness = 1,
                  sf::Color color = sf::Color::White);
    void drawArrow(float x1, float y1, float x2, float y2,
                   sf::Color color = sf::Color::White);
    void togglePause();
    void centralizeWindow();
};

#endif // GAME_HPP
