#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <sstream>

const int CELL_SIZE = 30;

class flyingObject {
public:
    std::vector<std::vector<int>> forme;
    int x_;
    int y_;
    int type_;

    flyingObject() : x_(5), y_(1), type_(1) {}

    void rotate_right_piece() {
        std::vector<std::vector<int>> new_forme(forme[0].size(), std::vector<int>(forme.size(), 0));
        for (int i = 0; i < forme.size(); ++i) {
            for (int j = 0; j < forme[0].size(); ++j) {
                new_forme[j][forme.size() - 1 - i] = forme[i][j];
            }
        }
        forme = new_forme;
    }

    void rotate_left_piece() {
        std::vector<std::vector<int>> new_forme(forme[0].size(), std::vector<int>(forme.size(), 0));
        for (int i = 0; i < forme.size(); ++i) {
            for (int j = 0; j < forme[0].size(); ++j) {
                new_forme[forme[0].size() - 1 - j][i] = forme[i][j];
            }
        }
        forme = new_forme;
    }
};

class Board {
public:
    const int width_;
    const int height_;
    std::vector<std::vector<int>> grid_;
    int score_;
    int fallInterval_; 

    Board(int width, int height)
        : width_(width), height_(height), grid_(height, std::vector<int>(width, 0)), score_(0), fallInterval_(600) {}

    void add_walls() {
        for (int i = 0; i < width_; i++) {
            grid_[0][i] = -1;
            grid_[height_ - 1][i] = -1;
        }
        for (int i = 0; i < height_; i++) {
            grid_[i][0] = -1;
            grid_[i][width_ - 1] = -1;
        }
    }

    void setPiece(const flyingObject& piece) {
        for (int i = 0; i < piece.forme.size(); ++i) {
            for (int j = 0; j < piece.forme[0].size(); ++j) {
                if (piece.forme[i][j] == 1) {
                    grid_[piece.y_ + i][piece.x_ + j] = piece.type_;
                }
            }
        }
    }

    void removePiece(const flyingObject& piece) {
        for (int i = 0; i < piece.forme.size(); ++i) {
            for (int j = 0; j < piece.forme[0].size(); ++j) {
                if (piece.forme[i][j] == 1) {
                    grid_[piece.y_ + i][piece.x_ + j] = 0;
                }
            }
        }
    }

    bool isCollision(const flyingObject& piece) {
        for (int i = 0; i < piece.forme.size(); ++i) {
            for (int j = 0; j < piece.forme[0].size(); ++j) {
                if (piece.forme[i][j] == 1) {
                    int newX = piece.x_ + j;
                    int newY = piece.y_ + i;
                    if (newX < 0 || newX >= width_ || newY < 0 || newY >= height_ || grid_[newY][newX] != 0) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void turn_right(flyingObject& piece) {
        removePiece(piece);
        piece.rotate_right_piece();
        if (!isCollision(piece)) {
            setPiece(piece);
        } else {
            piece.rotate_left_piece();
            setPiece(piece);
        }
    }

    void turn_left(flyingObject& piece) {
        removePiece(piece);
        piece.rotate_left_piece();
        if (!isCollision(piece)) {
            setPiece(piece);
        } else {
            piece.rotate_right_piece();
            setPiece(piece);
        }
    }

    bool move_down(flyingObject& piece) {
        removePiece(piece);
        piece.y_++;
        if (!isCollision(piece)) {
            setPiece(piece);
            return true;
        } else {
            piece.y_--;
            setPiece(piece);
            return false;
        }
    }

    void clearLines() {
        int linesCleared = 0;
        for (int i = height_ - 2; i >= 0; --i) {
            bool lineComplete = true;
            for (int j = 1; j < width_ - 1; ++j) {
                if (grid_[i][j] == 0) {
                    lineComplete = false;
                    break;
                }
            }
            if (lineComplete) {
                for (int j = 1; j < width_ - 1; ++j) {
                    grid_[i][j] = 0;
                }
                for (int k = i; k > 0; --k) {
                    for (int j = 1; j < width_ - 1; ++j) {
                        grid_[k][j] = grid_[k - 1][j];
                    }
                }
                i++; 
                linesCleared++;
            }
        }
        if (linesCleared > 0) {
            score_ += linesCleared * 100;
            fallInterval_ = std::max(200, fallInterval_ - 50); 
        }
    }

    void draw(sf::RenderWindow& window) {
        for (int i = 0; i < height_; ++i) {
            for (int j = 0; j < width_; ++j) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

                if (grid_[i][j] == 0) {
                    cell.setFillColor(sf::Color::Black); // Empty cell
                } else if (grid_[i][j] == -1) {
                    cell.setFillColor(sf::Color::White); // Wall
                } else {
                    // Assign colors based on the type of the piece
                    switch (grid_[i][j]) {
                        case 1: cell.setFillColor(sf::Color::Cyan); break;    // I-block
                        case 2: cell.setFillColor(sf::Color::Yellow); break;  // O-block
                        case 3: cell.setFillColor(sf::Color::Magenta); break; // T-block
                        case 4: cell.setFillColor(sf::Color::Blue); break;    // L-block
                        case 5: cell.setFillColor(sf::Color::Green); break;   // J-block
                        case 6: cell.setFillColor(sf::Color::Red); break;     // S-block
                        case 7: cell.setFillColor(sf::Color(255, 165, 0)); break; // Z-block (Orange)
                        default: cell.setFillColor(sf::Color::Black); break;
                    }
                }

                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::White);
                window.draw(cell);
            }
        }
    }
};

// Pièce en forme de "I"
class IBlock : public flyingObject {
public:
    IBlock() {
        forme = {
            {1, 1, 1, 1}
        };
        x_ = 4; // Position initiale en X
        y_ = 1; // Position initiale en Y
        type_ = 1;
    }
};

// Pièce en forme de "O"
class OBlock : public flyingObject {
public:
    OBlock() {
        forme = {
            {1, 1},
            {1, 1}
        };
        x_ = 4;
        y_ = 1;
        type_ = 2;
    }
};

// Pièce en forme de "T"
class TBlock : public flyingObject {
public:
    TBlock() {
        forme = {
            {0, 1, 0},
            {1, 1, 1}
        };
        x_ = 4;
        y_ = 1;
        type_ = 3;
    }
};

// Pièce en forme de "L"
class LBlock : public flyingObject {
public:
    LBlock() {
        forme = {
            {1, 0},
            {1, 0},
            {1, 1}
        };
        x_ = 4;
        y_ = 1;
        type_ = 4;
    }
};

// Pièce en forme de "J"
class JBlock : public flyingObject {
public:
    JBlock() {
        forme = {
            {0, 1},
            {0, 1},
            {1, 1}
        };
        x_ = 4;
        y_ = 1;
        type_ = 5;
    }
};

// Pièce en forme de "S"
class SBlock : public flyingObject {
public:
    SBlock() {
        forme = {
            {0, 1, 1},
            {1, 1, 0}
        };
        x_ = 4;
        y_ = 1;
        type_ = 6;
    }
};

// Pièce en forme de "Z"
class ZBlock : public flyingObject {
public:
    ZBlock() {
        forme = {
            {1, 1, 0},
            {0, 1, 1}
        };
        x_ = 4;
        y_ = 1;
        type_ = 7;
    }
};

flyingObject* generateRandomPiece() {
    int random = rand() % 7;
    switch (random) {
        case 0: return new IBlock();
        case 1: return new OBlock();
        case 2: return new TBlock();
        case 3: return new LBlock();
        case 4: return new JBlock();
        case 5: return new SBlock();
        case 6: return new ZBlock();
    }
    return new IBlock();
}

void drawNextPiece(sf::RenderWindow& window, flyingObject* nextPiece, int startX, int startY) {
    for (int i = 0; i < nextPiece->forme.size(); ++i) {
        for (int j = 0; j < nextPiece->forme[0].size(); ++j) {
            if (nextPiece->forme[i][j] == 1) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(startX + j * CELL_SIZE, startY + i * CELL_SIZE);

                switch (nextPiece->type_) {
                    case 1: cell.setFillColor(sf::Color::Cyan); break;    // I-block
                    case 2: cell.setFillColor(sf::Color::Yellow); break;  // O-block
                    case 3: cell.setFillColor(sf::Color::Magenta); break; // T-block
                    case 4: cell.setFillColor(sf::Color::Blue); break;    // L-block
                    case 5: cell.setFillColor(sf::Color::Green); break;   // J-block
                    case 6: cell.setFillColor(sf::Color::Red); break;     // S-block
                    case 7: cell.setFillColor(sf::Color(255, 165, 0)); break; // Z-block (Orange)
                    default: cell.setFillColor(sf::Color::Black); break;
                }

                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::White);
                window.draw(cell);
            }
        }
    }
}

int main() {
    const int boardWidth = 12;
    const int boardHeight = 20;

    sf::RenderWindow window(sf::VideoMode(boardWidth * CELL_SIZE + 200, boardHeight * CELL_SIZE), "Tetris with SFML");
    window.setFramerateLimit(60);

    Board board(boardWidth, boardHeight);
    board.add_walls();
    flyingObject* currentPiece = generateRandomPiece();
    flyingObject* nextPiece = generateRandomPiece();
    board.setPiece(*currentPiece);

    bool gameOver = false;
    auto lastFall = std::chrono::steady_clock::now();
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Left) {
                    board.removePiece(*currentPiece);
                    currentPiece->x_--;
                    if (board.isCollision(*currentPiece)) {
                        currentPiece->x_++;
                    }
                    board.setPiece(*currentPiece);
                } else if (event.key.code == sf::Keyboard::Right) {
                    board.removePiece(*currentPiece);
                    currentPiece->x_++;
                    if (board.isCollision(*currentPiece)) {
                        currentPiece->x_--;
                    }
                    board.setPiece(*currentPiece);
                } else if (event.key.code == sf::Keyboard::Down) {
                    if (!board.move_down(*currentPiece)) {
                        board.clearLines();
                        delete currentPiece;
                        currentPiece = nextPiece;
                        nextPiece = generateRandomPiece();
                        if (board.isCollision(*currentPiece)) {
                            gameOver = true;
                        }
                    }
                } else if (event.key.code == sf::Keyboard::Up) {
                    board.turn_left(*currentPiece);
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFall).count() >= board.fallInterval_) {
            if (!board.move_down(*currentPiece)) {
                board.clearLines();
                delete currentPiece;
                currentPiece = nextPiece;
                nextPiece = generateRandomPiece();
                if (board.isCollision(*currentPiece)) {
                    gameOver = true;
                }
            }
            lastFall = now;
        }

        window.clear();

        board.draw(window);

        drawNextPiece(window, nextPiece, boardWidth * CELL_SIZE + 50, 100);

        sf::Font font;
        if (!font.loadFromFile("BigShouldersStencil_24pt-Bold.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString("Score: " + std::to_string(board.score_));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(boardWidth * CELL_SIZE + 50, 300);
        window.draw(scoreText);

        window.display();

        if (gameOver) {
            std::cout << "Game Over! Final Score: " << board.score_ << std::endl;
            window.close();
        }
    }

    delete currentPiece;
    delete nextPiece;
    return 0;
}