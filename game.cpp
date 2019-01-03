#include "game.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

Game::Game():
      mWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), TITLE_OF_MAIN_WINDOW,
              sf::Style::Close)
{
    mWindow.setFramerateLimit(FPS);
    centralizeWindow();
    mFont.loadFromFile("res/fonts/BRLNSR.TTF");
    initMap();
    createConnections(Connectivity::FOUR);
    mStart = &mMap[1][1];
    mEnd = &mMap[5][5];
    mStart->distance = 0;
    mOldWave.insert(mStart);
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (mWindow.isOpen())
    {
        auto elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        while (timeSinceLastUpdate > mFrameTime) {
            timeSinceLastUpdate -= mFrameTime;
            inputPhase();
            updatePhase(mFrameTime);
        }
        inputPhase();
        updatePhase(mFrameTime);
        renderPhase();
    }
}

void Game::restart()
{
    resetMap();
    mOldWave.clear();
    mStart->distance = 0;
    mOldWave.insert(mStart);
    mCurrentWave.clear();
    mAlgorythmFinished = false;
    mPathRestored = false;
    mPathFound = false;
    mShortestPath.clear();
    mMaxDist = 0;
}

void Game::initMap()
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            mMap[y][x].x = x;
            mMap[y][x].y = y;

            if(x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1)
            {
                mMap[y][x].obstacle = true;
                mMap[y][x].distance = OBSTACLE;
            } else {
                mMap[y][x].obstacle = false;
                mMap[y][x].distance = UNVISITED;
            }
        }
    }
}

void Game::resetMap()
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            mMap[y][x].distance = mMap[y][x].obstacle ? OBSTACLE : UNVISITED;
            mMap[y][x].vx = 0;
            mMap[y][x].vy = 0;
        }
    }
    createConnections(Connectivity::FOUR);
}

void Game::createConnections(Game::Connectivity connectivity)
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            mMap[y][x].neighbours.clear();
            if(connectivity == Connectivity::FOUR)
            {
                const int dx[] = { 1, 0, -1, 0 };
                const int dy[] = { 0, 1, 0, -1 };
                for(int dir = 0; dir < 4; ++dir)
                {
                    int nx = x + dx[dir];
                    int ny = y + dy[dir];
                    if(isValidCoords(nx, ny))
                    {
                        mMap[y][x].neighbours.push_back(&mMap[ny][nx]);
                    }
                }
            }
            else if(connectivity == Connectivity::EIGHT)
            {
                for(int dx = -1; dx <= 1; ++dx)
                {
                    for(int dy = -1; dy <= 1; ++dy)
                    {
                        if(dx == 0 && dy == 0) continue;
                        int nx = x + dx;
                        int ny = y + dy;
                        if(isValidCoords(nx, ny))
                        {
                            mMap[y][x].neighbours.push_back(&mMap[ny][nx]);
                        }
                    }
                }
            }
        }
    }
}

void Game::restorePath()
{
    mShortestPath.clear();
    Node* current = mEnd;
    mShortestPath.push_back(mEnd);
    while (current != mStart) {
        auto neigbours = current->neighbours;
        neigbours.erase(std::remove_if(neigbours.begin(), neigbours.end(),
                     [](Node* node) { return node->obstacle; }), neigbours.end());
        std::sort(neigbours.begin(), neigbours.end()
                  , [](Node *lhs, Node *rhs){
            return lhs->distance < rhs->distance;
        });
        current = neigbours[0];
        mShortestPath.push_back(current);
    }
    std::reverse(mShortestPath.begin(), mShortestPath.end());
}

void Game::calcGradients()
{
    if(mPathFound)
    {
        for(int y = 0; y < MAP_HEIGHT; ++y)
        {
            for(int x = 0; x < MAP_WIDTH; ++x)
            {
                float distLeft = isValidCoords(x - 1, y) && !mMap[y][x - 1].obstacle ? mMap[y][x - 1].distance : mMap[y][x].distance;
                float distRight = isValidCoords(x + 1, y) && !mMap[y][x + 1].obstacle ? mMap[y][x + 1].distance : mMap[y][x].distance;
                float distUp = isValidCoords(x, y - 1) && !mMap[y - 1][x].obstacle ? mMap[y - 1][x].distance : mMap[y][x].distance;
                float distDown = isValidCoords(x, y + 1) && !mMap[y + 1][x].obstacle ? mMap[y + 1][x].distance : mMap[y][x].distance;
                float Dx = distRight - distLeft;
                float Dy = distDown - distUp;
                float r = sqrtf(Dx * Dx + Dy * Dy);
                mMap[y][x].vx = Dx / r;
                mMap[y][x].vy = Dy / r;
            }
        }
    }
}

bool Game::isValidCoords(int x, int y) const
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

void Game::inputPhase()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            mWindow.close();
        }
        else if(event.type == sf::Event::MouseButtonReleased)
        {
            int mx = event.mouseButton.x / CELL_SIZE;
            int my = event.mouseButton.y / CELL_SIZE;

            if(event.mouseButton.button == sf::Mouse::Left)
            {
                mMap[my][mx].obstacle = false;
                mStart = &mMap[my][mx];
            }
            else if(event.mouseButton.button == sf::Mouse::Right)
            {
                mMap[my][mx].obstacle = false;
                mEnd = &mMap[my][mx];
            }
            else if(event.mouseButton.button == sf::Mouse::Middle)
            {
                if(&mMap[my][mx] != mStart && &mMap[my][mx] != mEnd)
                {
                    mMap[my][mx].obstacle = !mMap[my][mx].obstacle;
                }
            }
            restart();
        }
        else if(event.type == sf::Event::KeyReleased)
        {
            if(event.key.code == sf::Keyboard::Return)
            {
                restart();
            }
            else if(event.key.code == sf::Keyboard::Space)
            {
                togglePause();
            }
        }
    }
}

void Game::updatePhase(sf::Time frameTime)
{
    if(mIsPaused) return;
    if(!mAlgorythmFinished && !mOldWave.empty())
    {
        mCurrentWave.clear();
        for(auto &node: mOldWave)
        {
            auto neighbours = node->neighbours;
            for(auto &neighbour: neighbours)
            {
                if(neighbour->distance == UNVISITED)
                {
                    neighbour->distance = node->distance + 1;
                    mCurrentWave.insert(neighbour);
                    if(mMaxDist < neighbour->distance)
                    {
                        mMaxDist = neighbour->distance;
                    }
                }
            }
        }
        mOldWave.clear();
        for(auto &node: mCurrentWave)
        {
            mOldWave.insert(node);
        }
    }
    else
    {
        mAlgorythmFinished = true;
        if(mEnd->distance != UNVISITED)
        {
            mPathFound = true;
        }
    }
    if(mPathFound && !mPathRestored)
    {
        restorePath();
        calcGradients();
    }
}

void Game::renderPhase()
{
    mWindow.clear();

    drawMap();
    drawCurrentWave();
    if(mPathFound)
    {
        drawPath();
        drawGradients();
    }
    mWindow.display();
}

void Game::drawMap()
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            sf::RectangleShape node;
            node.setPosition(mMap[y][x].x * CELL_SIZE + BORDER_SIZE,
                             mMap[y][x].y * CELL_SIZE + BORDER_SIZE);
            node.setSize({ CELL_SIZE - BORDER_SIZE, CELL_SIZE - BORDER_SIZE });

            float blue = float(mMap[y][x].distance / mMaxDist);
            node.setFillColor(sf::Color(0, 0, int(blue * 255.f)));

            if(mMap[y][x].obstacle)
            {
                node.setFillColor(sf::Color::White);
            }
            if(&mMap[y][x] == mStart)
            {
                node.setFillColor(sf::Color::Magenta);
            }
            if(&mMap[y][x] == mEnd)
            {
                node.setFillColor(sf::Color::Red);
            }
            mWindow.draw(node);
            auto val = mMap[y][x].distance == OBSTACLE ? -1 : mMap[y][x].distance;
            sf::Text text(std::to_string(val), mFont, 14);
            text.setFillColor(sf::Color::White);
            text.setPosition(mMap[y][x].x * CELL_SIZE + BORDER_SIZE, mMap[y][x].y * CELL_SIZE);
            mWindow.draw(text);
        }
    }
}

void Game::drawCurrentWave()
{
    //Draw current wave
    for(auto &node: mCurrentWave)
    {
        sf::RectangleShape rect;
        rect.setPosition(node->x * CELL_SIZE + BORDER_SIZE,
                         node->y * CELL_SIZE + BORDER_SIZE);
        rect.setSize({ CELL_SIZE - BORDER_SIZE, CELL_SIZE - BORDER_SIZE });
        rect.setFillColor(sf::Color::Green);
        mWindow.draw(rect);
    }
}

void Game::drawGradients()
{
    //Draw vectors
    const int arrowLength = CELL_SIZE / 2;
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            float x1 = mMap[y][x].x * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2;
            float y1 = mMap[y][x].y * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2;
            float angle = atan2f(mMap[y][x].vy, mMap[y][x].vx);
            float x2 = x1 + arrowLength * cosf(angle);
            float y2 = y1 + arrowLength * sinf(angle);
            drawArrow(x1, y1, x2, y2, sf::Color::Cyan);
        }
    }
}

void Game::drawPath()
{
    for(size_t i = 1; i < mShortestPath.size(); ++i)
    {
        auto next = mShortestPath[i];
        auto curr = mShortestPath[i - 1];
        sf::CircleShape spot;
        spot.setRadius(0.2f * CELL_SIZE);
        spot.setPosition(curr->x * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2,
                         curr->y * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2);
        spot.setOrigin(spot.getRadius(), spot.getRadius());
        spot.setFillColor(sf::Color::Yellow);
        mWindow.draw(spot);
        spot.setPosition(next->x * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2,
                         next->y * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2);
        mWindow.draw(spot);
        drawLine(curr->x * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2,
                 curr->y * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2 + 3,
                 next->x * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2,
                 next->y * CELL_SIZE + CELL_SIZE / 2 + BORDER_SIZE / 2 + 3,
                 6, sf::Color::Yellow);
    }
}

void Game::drawLine(float x1, float y1, float x2, float y2, float thickness, sf::Color color)
{
    auto dx = x2 - x1;
    auto dy = y2 - y1;
    auto angle = atan2(dy, dx) / M_PI * 180;
    auto length = sqrtf(dx * dx + dy * dy);
    sf::RectangleShape line;
    line.setPosition(x1, y1 - thickness / 2);
    line.setSize(sf::Vector2f(length, thickness));
    line.setOrigin(0, +thickness / 2);
    line.setRotation(angle);
    line.setFillColor(color);
    mWindow.draw(line);
}

void Game::drawArrow(float x1, float y1, float x2, float y2, sf::Color color)
{
    auto dx = x2 - x1;
    auto dy = y2 - y1;
    auto angle = atan2(dy, dx);
    auto leftX = x2 - 10 * cos(angle - M_PI / 12);
    auto leftY = y2 - 10 * sin(angle - M_PI / 12);
    auto rightX = x2 - 10 * cos(angle + M_PI / 12);
    auto rightY = y2 - 10 * sin(angle + M_PI / 12);
    drawLine(x1, y1, x2, y2, 1, color);
    drawLine(x2, y2, leftX, leftY, 1, color);
    drawLine(x2, y2, rightX, rightY, 1, color);
}

void Game::togglePause()
{
    mIsPaused = !mIsPaused;
}

void Game::centralizeWindow()
{
    const int screenWidth = sf::VideoMode::getDesktopMode().width;
    const int screenHeight = sf::VideoMode::getDesktopMode().height;
    mWindow.setPosition(sf::Vector2i((screenWidth - SCREEN_WIDTH) / 2,
                                         (screenHeight - SCREEN_HEIGHT) / 2));
}
