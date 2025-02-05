#include <algorithm>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <sstream>

using namespace std;

// ----------------------------------------------------
// Forward declarations
// ----------------------------------------------------
template <typename T>
class AVLNode;

template <typename T>
class AVLTree;

// Global SFML Window pointer (used by animation).
sf::RenderWindow* globalWindowPtr = nullptr;

// Global Font for SFML text.
sf::Font globalFont;

// ----------------------------------------------------
// AVL Node Definition
// ----------------------------------------------------
template <typename T>
class AVLNode {
public:
    T key;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(T k)
        : key(k), left(nullptr), right(nullptr), height(1)
    {}
};

// ----------------------------------------------------
// "Special AVL" Tree
//   - Maintains a sorted vector of keys
//   - Rebuilds a perfectly balanced tree on each insert
// ----------------------------------------------------
template <typename T>
class AVLTree {
private:
    AVLNode<T>* root;
    vector<T> sortedElements; // Always keeps keys in sorted order

    // Compute the node's height
    int height(AVLNode<T>* node) {
        return (node == nullptr) ? 0 : node->height;
    }

    // Build a perfectly balanced BST from sortedElements[start..end]
    // For an even count of elements, pick the "upper" middle:
    //    mid = (start + end + 1) / 2
    AVLNode<T>* buildBalancedTree(int start, int end) {
        if (start > end) {
            return nullptr;
        }

        int mid = (start + end + 1) / 2; // "upper" middle
        AVLNode<T>* node = new AVLNode<T>(sortedElements[mid]);

        node->left  = buildBalancedTree(start, mid - 1);
        node->right = buildBalancedTree(mid + 1, end);

        int lh = height(node->left);
        int rh = height(node->right);
        node->height = 1 + std::max(lh, rh);

        return node;
    }

    // Insert into the sorted vector (if not a duplicate), then rebuild
    AVLNode<T>* insertRebuild(T key) {
        auto it = std::lower_bound(sortedElements.begin(), sortedElements.end(), key);
        if (it == sortedElements.end() || *it != key) {
            sortedElements.insert(it, key);
        }
        return buildBalancedTree(0, (int)sortedElements.size() - 1);
    }

    // Remove from the sorted vector (if present), then rebuild
    AVLNode<T>* deleteRebuild(T key) {
        auto it = std::lower_bound(sortedElements.begin(), sortedElements.end(), key);
        if (it != sortedElements.end() && *it == key) {
            sortedElements.erase(it);
        }
        if (sortedElements.empty()) {
            return nullptr;
        }
        return buildBalancedTree(0, (int)sortedElements.size() - 1);
    }

    // Standard BST search
    bool searchBST(AVLNode<T>* node, T key) {
        if (!node) {
            return false;
        }
        if (node->key == key) {
            return true;
        }
        if (key < node->key) {
            return searchBST(node->left, key);
        } else {
            return searchBST(node->right, key);
        }
    }

    // For debugging: In-order traversal
    void inorder(AVLNode<T>* node) {
        if (node) {
            inorder(node->left);
            cout << node->key << " ";
            inorder(node->right);
        }
    }

public:
    AVLTree() : root(nullptr) {}

    // Public Insert
    void insert(T key) {
        root = insertRebuild(key);
    }

    // Public Remove
    void remove(T key) {
        root = deleteRebuild(key);
    }

    // Public Search
    bool search(T key) {
        return searchBST(root, key);
    }

    // Print Inorder
    void printInorder() {
        inorder(root);
        cout << endl;
    }

    // Access the root (for drawing, etc.)
    AVLNode<T>* getRoot() {
        return root;
    }

    // Return the path (node pointers) visited during a search for "key"
    // This is used for highlighting the path in the SFML drawing.
    vector<AVLNode<T>*> getSearchPath(T key) {
        vector<AVLNode<T>*> path;
        AVLNode<T>* current = root;
        while (current) {
            path.push_back(current);
            if (current->key == key) {
                break;
            }
            else if (key < current->key) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }
        return path;
    }
};

// ----------------------------------------------------
// Utility to check if a node is in the search path
// ----------------------------------------------------
bool isNodeInPath(AVLNode<int>* node, const vector<AVLNode<int>*>& path) {
    for (auto* p : path) {
        if (p == node) {
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------
// Recursive SFML Drawing with optional path highlight
// ----------------------------------------------------
void drawTree(sf::RenderWindow &window,
              AVLNode<int>* node,
              float x,
              float y,
              float horizontalOffset,
              const vector<AVLNode<int>*>& searchPath)
{
    if (!node) return;

    const float radius = 30.f;
    bool highlight = isNodeInPath(node, searchPath);

    // Draw the node circle
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(x, y);

    if (highlight) {
        circle.setFillColor(sf::Color::Red);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(3);
    } else {
        circle.setFillColor(sf::Color::Yellow);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(3);
    }

    // Node text
    sf::Text text;
    text.setFont(globalFont);
    text.setString(std::to_string(node->key));
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);

    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f,
                   textRect.top + textRect.height / 2.0f);
    text.setPosition(x, y);

    float verticalSpacing = 100.f;

    // Draw left child edge
    if (node->left) {
        float childX = x - horizontalOffset;
        float childY = y + verticalSpacing;

        bool childHighlight = highlight && isNodeInPath(node->left, searchPath);

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, y + radius),
                       childHighlight ? sf::Color::Red : sf::Color::Yellow),
            sf::Vertex(sf::Vector2f(childX, childY - radius),
                       childHighlight ? sf::Color::Red : sf::Color::Yellow)
        };
        window.draw(line, 2, sf::Lines);

        drawTree(window, node->left, childX, childY, horizontalOffset / 2, searchPath);
    }

    // Draw right child edge
    if (node->right) {
        float childX = x + horizontalOffset;
        float childY = y + verticalSpacing;

        bool childHighlight = highlight && isNodeInPath(node->right, searchPath);

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, y + radius),
                       childHighlight ? sf::Color::Red : sf::Color::Yellow),
            sf::Vertex(sf::Vector2f(childX, childY - radius),
                       childHighlight ? sf::Color::Red : sf::Color::Yellow)
        };
        window.draw(line, 2, sf::Lines);

        drawTree(window, node->right, childX, childY, horizontalOffset / 2, searchPath);
    }

    // Finally draw the circle and the text
    window.draw(circle);
    window.draw(text);
}

// ----------------------------------------------------
// Animation function (for Insert or Search tasks)
// ----------------------------------------------------
void animateTask(sf::RenderWindow &window,
                 const std::string &message,
                 float duration,
                 AVLTree<int>& tree,
                 const vector<AVLNode<int>*>& searchPath = {})
{
    sf::Clock clock;
    while (clock.getElapsedTime().asSeconds() < duration) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);

        // Draw the tree, highlighting the given searchPath (if any).
        drawTree(window, tree.getRoot(),
                 window.getSize().x / 2.f, 50.f,
                 300.f, searchPath);

        // Draw the message in the bottom-left corner.
        sf::Text taskText;
        taskText.setFont(globalFont);
        taskText.setString(message);
        taskText.setCharacterSize(28);
        taskText.setFillColor(sf::Color::White);
        taskText.setStyle(sf::Text::Bold);
        taskText.setPosition(10.f, window.getSize().y - 50.f);
        window.draw(taskText);

        window.display();
    }
}

// ----------------------------------------------------
// Main
// ----------------------------------------------------
int main() {
    // Initial array of elements to insert
    int elements[] = {
        15, 23, 29, 33, 37, 41, 44, 49, 52, 54,
        60, 62, 68, 70, 75, 85, 90, 95, 100, 110
    };
    int numElements = static_cast<int>(sizeof(elements) / sizeof(elements[0]));
    int insertionIndex = 0;

    AVLTree<int> avl;

    // Load the font for drawing
    if (!globalFont.loadFromFile("ArialTh.ttf")) {
        std::cout << "Error loading font 'ArialTh.ttf'" << std::endl;
        return -1;
    }

    // Create the SFML window
    sf::RenderWindow window(sf::VideoMode(1600, 1000),
                            "AVL Tree Visualization (Binary Search-Like)");
    globalWindowPtr = &window;

    // Delay between automatic insertions for initial array
    const float insertionDelay = 2.0f;
    sf::Clock insertionClock;

    // When we're done inserting the initial array, let user do interactive insert/search
    bool initialTreeComplete = false;

    // Insert & Search text boxes: state
    sf::String userInputInsert;
    sf::String userInputSearch;
    bool isTypingInsert = false;
    bool isTypingSearch = false;

    // Two rectangles for the text boxes (bottom-right corner)
    auto getInsertBoxRect = [&](sf::RenderWindow &win) {
        return sf::IntRect(win.getSize().x - 610, win.getSize().y - 50, 300, 50);
    };
    auto getSearchBoxRect = [&](sf::RenderWindow &win) {
        return sf::IntRect(win.getSize().x - 300, win.getSize().y - 50, 300, 50);
    };

    // Main Loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Once the initial array is fully inserted, handle user input
            if (initialTreeComplete) {
                if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::IntRect insertRect = getInsertBoxRect(window);
                    sf::IntRect searchRect = getSearchBoxRect(window);

                    // Check if click is in "Insert" box
                    if (insertRect.contains(mousePos)) {
                        isTypingInsert = true;
                        isTypingSearch = false;
                    }
                    // Or check if click is in "Search" box
                    else if (searchRect.contains(mousePos)) {
                        isTypingInsert = false;
                        isTypingSearch = true;
                    }
                    else {
                        // Click outside both
                        isTypingInsert = false;
                        isTypingSearch = false;
                    }
                }

                // Typing in the active box
                if (event.type == sf::Event::TextEntered) {
                    char c = static_cast<char>(event.text.unicode);

                    if (c == '\r') {
                        // Enter pressed
                        if (isTypingInsert && !userInputInsert.isEmpty()) {
                            int newVal = atoi(userInputInsert.toAnsiString().c_str());
                            animateTask(window, "Inserting " + std::to_string(newVal), 1.0f, avl);
                            avl.insert(newVal);
                            userInputInsert.clear();
                        }
                        else if (isTypingSearch && !userInputSearch.isEmpty()) {
                            int searchVal = atoi(userInputSearch.toAnsiString().c_str());
                            auto path = avl.getSearchPath(searchVal);

                            bool found = (!path.empty() && path.back()->key == searchVal);
                            std::string msg = (found ? "Found " : "Not Found ") + std::to_string(searchVal);
                            animateTask(window, msg, 2.0f, avl, path);

                            userInputSearch.clear();
                        }
                    }
                    else if (c == 8) {
                        // Backspace
                        if (isTypingInsert && !userInputInsert.isEmpty()) {
                            userInputInsert.erase(userInputInsert.getSize() - 1, 1);
                        }
                        else if (isTypingSearch && !userInputSearch.isEmpty()) {
                            userInputSearch.erase(userInputSearch.getSize() - 1, 1);
                        }
                    }
                    else {
                        // Accept digits, minus sign, etc.
                        if ((c >= '0' && c <= '9') || c == '-') {
                            if (isTypingInsert) {
                                userInputInsert += c;
                            }
                            else if (isTypingSearch) {
                                userInputSearch += c;
                            }
                        }
                    }
                }
            } // end if (initialTreeComplete)
        }

        // Automatically insert from the initial array
        if (!initialTreeComplete && insertionIndex < numElements) {
            if (insertionClock.getElapsedTime().asSeconds() >= insertionDelay) {
                std::string taskMsg = "Inserting " + std::to_string(elements[insertionIndex]);
                animateTask(window, taskMsg, 1.0f, avl);
                avl.insert(elements[insertionIndex]);

                insertionIndex++;
                insertionClock.restart();

                if (insertionIndex == numElements) {
                    initialTreeComplete = true;
                }
            }
        }

        // Clear and draw
        window.clear(sf::Color::Black);

        // Always draw the tree (with no special path highlight by default).
        drawTree(window, avl.getRoot(),
                 window.getSize().x / 2.f, 50.f,
                 300.f, {});

        // If the tree is complete, allow user to see the text boxes
        if (initialTreeComplete) {
            // Insert box
            sf::RectangleShape insertBox;
            insertBox.setSize(sf::Vector2f(300.f, 50.f));
            sf::IntRect insRect = getInsertBoxRect(window);
            insertBox.setPosition((float)insRect.left, (float)insRect.top);
            insertBox.setFillColor(isTypingInsert
                                   ? sf::Color(50, 50, 200)
                                   : sf::Color(100, 100, 100));
            window.draw(insertBox);

            sf::Text insertText;
            insertText.setFont(globalFont);
            insertText.setString("Insert: " + userInputInsert);
            insertText.setCharacterSize(24);
            insertText.setFillColor(sf::Color::White);
            insertText.setPosition((float)insRect.left + 5, (float)insRect.top + 10);
            window.draw(insertText);

            // Search box
            sf::RectangleShape searchBox;
            searchBox.setSize(sf::Vector2f(300.f, 50.f));
            sf::IntRect seaRect = getSearchBoxRect(window);
            searchBox.setPosition((float)seaRect.left, (float)seaRect.top);
            searchBox.setFillColor(isTypingSearch
                                   ? sf::Color(50, 50, 200)
                                   : sf::Color(100, 100, 100));
            window.draw(searchBox);

            sf::Text searchText;
            searchText.setFont(globalFont);
            searchText.setString("Search: " + userInputSearch);
            searchText.setCharacterSize(24);
            searchText.setFillColor(sf::Color::White);
            searchText.setPosition((float)seaRect.left + 5, (float)seaRect.top + 10);
            window.draw(searchText);
        }

        window.display();
    }

    return 0;
}