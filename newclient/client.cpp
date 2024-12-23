#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "mail.h"
#include <algorithm>
#include <windows.h>
#include <filesystem>
#include <cstdlib> 
#include<thread>
#include <windows.h>
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
using namespace std;


const std::vector<std::string> optionTextShow = {
    "Show all apps", "Shutdown", "Webcam on", "Webcam off", "Take screenshot",
    "Get file", "List file", "Run app", "Close app", "List running apps",
    "Close app by PID", "Delete file", "List servers", "Change server"
};

int min1(int a, int b) {
    return (a < b) ? a : b;
}
int max1(int a, int b) {
    return (a > b) ? a : b;
}
const int SCREEN_WIDTH = 1786;
const int SCREEN_HEIGHT = 868;
const int MAX_TEXT_LENGTH = 20;
const int INITIAL_SIDEBAR_WIDTH = 50;
const int EXPANDED_SIDEBAR_WIDTH = INITIAL_SIDEBAR_WIDTH * 2;
const int TOGGLE_BUTTON_SIZE = 40;
int dem = 0;
string username, password;
enum Page {
    LOGIN,
    APP,
    REMOTE,
    HELP,
};
Page currentPage = LOGIN;
string int_to_string(int a)
{
    stringstream ss;
    ss << a;
    string res;
    ss >> res;
    return res;
}
struct ScrollBar {
    int yPosition;
    int height;
    int maxPosition;
};
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color, int maxWidth = 0) {
    std::string displayedText = text;
    if (maxWidth > 0) {
        int textWidth = 0, textHeight = 0;
        TTF_SizeText(font, displayedText.c_str(), &textWidth, &textHeight);
        while (textWidth > maxWidth && !displayedText.empty()) {
            displayedText.pop_back();
            TTF_SizeText(font, displayedText.c_str(), &textWidth, &textHeight);
        }
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, displayedText.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    int textWidth = surface->w;
    int textHeight = surface->h;
    SDL_Rect dstRect = { x, y, textWidth, textHeight };
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
struct Sidebar {
    int currentPosition;
    bool expanded;
    bool showBackButton;
};
void renderSidebar(SDL_Renderer* renderer, Sidebar& sidebar, TTF_Font* font) {
    // Draw sidebar background
    SDL_Rect sidebarRect = { 0, 0, sidebar.currentPosition + 11, SCREEN_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Màu đen
    SDL_RenderFillRect(renderer, &sidebarRect);

    const char* sidebarItems[] = { "Logout", "Remote", "Help" };
    int yPosition = 100;
    SDL_Color textColor = { 255, 215, 0 };  // Màu vàng

    // Render sidebar items if sidebar is expanded
    if (sidebar.currentPosition > INITIAL_SIDEBAR_WIDTH) {
        for (int i = 0; i < 3; ++i) {
            renderText(renderer, font, sidebarItems[i], 20, yPosition, textColor, sidebar.currentPosition - 20);
            yPosition += 60;
        }
        // Render Back button if showBackButton is true
        if (sidebar.showBackButton) {
            renderText(renderer, font, "Back", 20, yPosition, textColor, sidebar.currentPosition - 20);
        }
    }

    // Draw the toggle button
    SDL_Rect toggleButton = { 5, 0, TOGGLE_BUTTON_SIZE, TOGGLE_BUTTON_SIZE };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Màu đen
    SDL_RenderFillRect(renderer, &toggleButton);

    // Draw three horizontal lines (ngạch dậm) stacked vertically
    int lineWidth = 20;  // Chiều rộng của từng dấu ngạch dậm (mở rộng chiều rộng)
    int lineHeight = 7;  // Chiều cao của dấu ngạch dậm (giảm chiều cao để nằm ngang)
    int spacing = 2;  // Khoảng cách giữa các dấu ngạch dậm (càng nhỏ càng trồng lên nhau)

    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // Màu vàng

    // Vẽ ba dấu ngạch dậm nằm ngang và trồng lên nhau
    SDL_Rect line1 = { 5, 2, lineWidth + 20, lineHeight };  // Dấu ngạch dậm đầu tiên
    SDL_Rect line2 = { 5, 2 + lineHeight + 2 + spacing, lineWidth + 20, lineHeight };  // Dấu ngạch dậm thứ hai
    SDL_Rect line3 = { 5, 2 + 2 * (lineHeight + spacing) + 4, lineWidth + 20, lineHeight };  // Dấu ngạch dậm thứ ba


    // Vẽ các dấu ngạch dậm
    SDL_RenderFillRect(renderer, &line1);
    SDL_RenderFillRect(renderer, &line2);
    SDL_RenderFillRect(renderer, &line3);
}

// vẽ trang login 
void drawLoginPage(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* titleFont, const string& username, const string& password, bool registering, const string& message, SDL_Texture* backgroundTexture, bool usernameSelected, bool passwordSelected) {
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); // trắng

    SDL_Color textColor = { 255, 255, 255 };
    if(registering)
        renderText(renderer, titleFont,"REGISTER", 130, 180, textColor);
    else    
        renderText(renderer, titleFont, "LOGIN", 160, 180, textColor);

    renderText(renderer, font, "USERNAME:", 75, 255, textColor);

    SDL_Rect usernameBox = { 70, 248, 280, 35 };
    SDL_RenderDrawRect(renderer, &usernameBox);

    // Render username
    renderText(renderer, font, username, 180, 255, textColor, usernameBox.w - 10);

    // Hiển thị con trỏ trong ô username nếu được chọn
    if (usernameSelected) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 215, 0); // xanh
        SDL_RenderDrawRect(renderer, &usernameBox);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); // trắng
    renderText(renderer, font, "PASSWORD:", 75, 305, textColor);
    SDL_Rect passwordBox = { 70, 298, 280, 35 };
    SDL_RenderDrawRect(renderer, &passwordBox);

    // Render hidden password
    string hiddenPassword(password.size(), '*');
    renderText(renderer, font, hiddenPassword, 180, 308, textColor, passwordBox.w - 10);

    // Hiển thị con trỏ trong ô password nếu được chọn
    if (passwordSelected) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 215, 0); // xanh
        SDL_RenderDrawRect(renderer, &passwordBox);
    }

    SDL_SetRenderDrawColor(renderer, 13, 110, 253, 255);
    SDL_Rect actionButton = { 70, 380, 280, 40 };
    SDL_RenderFillRect(renderer, &actionButton);
    if(registering)
        renderText(renderer, font, "REGISTER", 163, 390, textColor);
    else    
        renderText(renderer, font, "LOGIN", 179, 390, textColor);

    if (!registering) {
        SDL_SetRenderDrawColor(renderer, 250, 60, 25, 255);
        SDL_Rect registerButton = { 70, 430, 280, 40 };
        SDL_RenderFillRect(renderer, &registerButton);
        renderText(renderer, font, "REGISTER", 165, 441, textColor);
    }

    renderText(renderer, font, message, 70, 480, textColor);
}

// vẽ trang  app 
void drawAppPage(SDL_Renderer* renderer, TTF_Font* font, Sidebar& sidebar, SDL_Texture* backgroundTexture) {
    SDL_RenderClear(renderer);
    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }

    sidebar.showBackButton = false;
    renderSidebar(renderer, sidebar, font);

    SDL_Color textColor = { 255, 255, 255 };
    int textOffset = sidebar.currentPosition;

    //renderText(renderer, font, "Welcome to the App!", 100 + textOffset, 100, textColor);
    //renderText(renderer, font, "This is your main dashboard.", 70 + textOffset, 200, textColor);
}
void drawHelpPage(SDL_Renderer* renderer, TTF_Font* font, Sidebar& sidebar, SDL_Texture* backgroundTexture) {
    SDL_RenderClear(renderer);
    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }

    sidebar.showBackButton = true;
    renderSidebar(renderer, sidebar, font);

    SDL_Color textColor = { 255, 255, 255 };
    int textOffset = sidebar.currentPosition;

    renderText(renderer, font, "When connecting to the server via Remote, you will see a control interface", 130, 170, textColor);
    renderText(renderer, font, "for managing and operating the server remotely.You can select the", 130, 190, textColor);
    renderText(renderer, font, "appropriate options to interact with the server and enter the corresponding", 130, 210, textColor);
    renderText(renderer, font, "paths in the adjacent field.", 130, 230, textColor);
    renderText(renderer, font, "The available control modes include", 130, 250, textColor);
    renderText(renderer, font, "Show all apps: List all apps of the server", 130, 270, textColor);
    renderText(renderer, font, "Take screenshot: Take a screenshot of the server", 130, 290, textColor);
    renderText(renderer, font, "Shutdown: Shutdown the server", 130, 310, textColor);
    renderText(renderer, font, "Webcam on: Turn on the server's camera", 130, 330, textColor);
    renderText(renderer, font, "Webcam off: Turn off the server's camera", 130, 350, textColor);
    renderText(renderer, font, "Get file <path>: Get a file from the server", 130, 370, textColor);
    renderText(renderer, font, "List file <folder path>: Lists all the files in the folder", 130, 390, textColor);
    renderText(renderer, font, "Run app <app.exe>: Run an app", 130, 410, textColor);
    renderText(renderer, font, "List running apps: List all running apps", 130, 430, textColor);
    renderText(renderer, font, "Close app <app.exe> : Close an app", 130, 450, textColor);
    renderText(renderer, font, "Close app by PID <app PID>: Close an app with the corresponding PID", 130, 470, textColor);
    renderText(renderer, font, "Change Server<id>: Switches to another server for control", 130, 490, textColor);
    renderText(renderer, font, "Delete<file>: Delete file of the server", 130, 510, textColor);
    renderText(renderer, font, "List Servers: List all servers ", 130, 530, textColor);
}
void drawSubmitButton(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Rect submitButton = { 560, 500, 180, 50 };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // màu đỏ
    SDL_RenderFillRect(renderer, &submitButton);
    renderText(renderer, font, " SEND", 620, 516, { 255, 255, 255 });
}
void drawButton(SDL_Renderer* renderer, const SDL_Rect& rect, const std::string& text, TTF_Font* font, SDL_Color color, SDL_Color textColor) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);

    int textX = rect.x + (rect.w / 2) - (text.length() * 5);  // Approximation for centering text
    int textY = rect.y + (rect.h / 2) - 10;                  // Approximation for centering text vertically
    renderText(renderer, font, text, textX, textY, textColor);
}

void drawRemotePage(SDL_Renderer* renderer, TTF_Font* font, Sidebar& sidebar,
    bool option1Selected, bool option2Enabled,
    const std::string& option2Title, ScrollBar& scrollBar,
    const std::string& processID, std::vector<std::string>& logMessages,
    const std::vector<int>& option, const std::string& selectedOptionText,
    SDL_Texture* backgroundTexture, SDL_Texture* imageTexture,
    bool loadImage, int visibleLines, int logHeight, int& scrollPosition) {

    SDL_RenderClear(renderer);

    // Render background
    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }

    // Sidebar
    sidebar.showBackButton = true;
    renderSidebar(renderer, sidebar, font);

    // Header
    SDL_Color headerColor = { 255, 255, 255 };
    renderText(renderer, font, "Remote PC App", 120, 20, headerColor);

    // Log console
    SDL_Rect logConsole = { 120, 70, 670, 390 };

    if (loadImage && imageTexture) {
        SDL_RenderCopy(renderer, imageTexture, nullptr, &logConsole);
    }
    else {
        if (loadImage && imageTexture) {
            SDL_RenderCopy(renderer, imageTexture, nullptr, &logConsole);
        }
        else {
            // Set background color
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &logConsole);

            // Calculate total lines and adjust scroll position
            int totalLines = static_cast<int>(logMessages.size());
            scrollPosition = max1(0, min1(scrollPosition, totalLines - visibleLines));

            // Render visible lines
            for (int i = scrollPosition; i < min1(scrollPosition + visibleLines, totalLines); ++i) {
                int textY = logConsole.y + (i - scrollPosition) * logHeight; // Adjust Y position for each line
                renderText(renderer, font, logMessages[i], logConsole.x + 5, textY, { 0, 255, 0 });
            }
        }


    }

    SDL_Rect upButton = { 730, 56, 60, 30 };
    SDL_Rect downButton = { 660, 56, 70, 30 };
    SDL_Rect clear = { 590, 56, 70, 30 };

    drawButton(renderer, upButton, "Up", font, { 100, 100, 255, 255 }, { 255, 255, 255, 255 });
    drawButton(renderer, downButton, "Down", font, { 100, 100, 255, 255 }, { 255, 255, 255, 255 });
    drawButton(renderer, clear, " Clear", font, { 100, 100, 255, 255 }, { 255, 255, 255, 255 });


    // Type of Request Dropdown
    SDL_Rect typeRequestBox = { 130, 500, 180, 50 };
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_RenderFillRect(renderer, &typeRequestBox);
    renderText(renderer, font, "Option", 190, 516, { 255, 255, 255 });
    renderText(renderer, font, "Select option ", 173, 470, { 255, 255, 0 });

    // Process ID Box
    SDL_Rect pidBox = { 350, 500, 180, 50 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &pidBox);

    SDL_Color textColor = { 255, 255, 255 };

    // Xử lý đoạn văn bản 
    string displayText = processID;  // Đoạn văn bản cần hiển thị
    if(processID.length() > 20)
        displayText = processID.substr(processID.length() - 20);  // Lấy 20 ký tự cuối

    // Render đoạn văn bản đã được cắt (hiển thị 10 ký tự cuối cùng)
    renderText(renderer, font, displayText, 360, 516, textColor);

    // Dropdown menu
    if (option1Selected) {
        // Dropdown box dimensions
        SDL_Rect dropdownBox = { 218, 120, 465, 270 };  // Adjusted for the dropdown size
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);  // Background color (gray)
        SDL_RenderFillRect(renderer, &dropdownBox);  // Fill the background of the dropdown
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Border color (white)
        SDL_RenderDrawRect(renderer, &dropdownBox);  // Draw the border

        // Option texts for the dropdown
        const std::vector<std::string> optionTexts = {
            "Show all apps", "Shutdown", "Webcam on", "Webcam off", "Take screenshot",
            "Get file", "List file", "Run app", "Close app", "List running apps",
            "Close app by PID", "Delete file", "List servers", "Change server"
        };

        const int OPTION_WIDTH = 220;  // Width of each option box
        const int OPTION_HEIGHT = 35;  // Height of each option box
        const int MARGIN_X_LEFT = 222; // X position for the left column
        const int MARGIN_X_RIGHT = 462; // X position for the right column
        const int MARGIN_Y = 123;  // Initial Y position
        const int SPACING_Y = 38;  // Vertical spacing between options

        // Loop through each option and render it
        for (size_t i = 0; i < optionTexts.size(); ++i) {
            // Determine which column (left or right) the option should go in
            int posX = (i < 7) ? MARGIN_X_LEFT : MARGIN_X_RIGHT;

            // Calculate the Y position for the options (arranged vertically)
            int posY = MARGIN_Y + static_cast<int>(i % 7) * SPACING_Y;

            // Draw the option box
            SDL_Rect optionBox = { posX, posY, OPTION_WIDTH, OPTION_HEIGHT };
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);  // Option background color (gray)
            SDL_RenderFillRect(renderer, &optionBox);  // Fill the background
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Border color (white)
            SDL_RenderDrawRect(renderer, &optionBox);  // Draw the border

            // Render the text for each option
            renderText(renderer, font, optionTexts[i],
                posX + 5,  // X position of the text (left-aligned with padding)
                posY + 3,  // Y position of the text (centered in the box)
                { 255, 255, 255 });  // Text color (white)
        }
    }

    // Selected option display
    renderText(renderer, font, "Selected: " + selectedOptionText,
        350, 470, { 255, 255, 0 });

    drawSubmitButton(renderer, font);
    SDL_RenderPresent(renderer);
}

void handleEvents(SDL_Renderer* renderer, SDL_Event& event, bool& option1Selected,
    std::vector<int>& option, std::string& selectedOptionText,
    bool& loadImage, SDL_Rect& submitButton,
    std::string& processID, std::vector<std::string>& logMessages,
    std::string& tenfile, int& scrollPosition,
    int logHeight, int visibleLines, SDL_Texture*& imageTexture,
    bool& check, TTF_Font* font, Sidebar& sidebar,
    bool option2Enabled, const std::string& option2Title, ScrollBar& scrollBar, SDL_Texture* backgroundTexture, bool enteringPID) {

    static int dem = 0; // Message counterf

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;


        // Handle dropdown options
        if (option1Selected) {
            const std::vector<std::string> optionTexts = {
                "list_apps", "shutdown", "webcam_on", "webcam_off", "get_screenshot",
                "get_file", "list_files", "run_app", "close_app", "running_apps",
                "close_by_id", "delete_file", "servers_IP", "change_server",
            };

            const int OPTION_WIDTH = 220;  // Width of each option box
            const int OPTION_HEIGHT = 35;  // Height of each option box
            const int MARGIN_X_LEFT = 222; // X position for the left column
            const int MARGIN_X_RIGHT = 462; // X position for the right column
            const int MARGIN_Y = 123;  // Initial Y position
            const int SPACING_Y = 38; // Vertical spacing between options

            for (size_t i = 0; i < optionTexts.size(); ++i) {
                int posX = (i < 7) ? MARGIN_X_LEFT : MARGIN_X_RIGHT;
                int posY = MARGIN_Y + static_cast<int>(i % 7) * SPACING_Y;

                SDL_Rect optionBox = { posX, posY, OPTION_WIDTH, OPTION_HEIGHT };

                if (mouseX >= optionBox.x && mouseX <= optionBox.x + optionBox.w &&
                    mouseY >= optionBox.y && mouseY <= optionBox.y + optionBox.h) {
                    // Set the selected option to 1 and reset the others
                    std::fill(option.begin(), option.end(), 0);
                    option[i] = 1;  // Mark the selected option
                    selectedOptionText = optionTextShow[i];
                    option1Selected = false;  // Close dropdown after selection
                    loadImage = (optionTexts[i] == "get_screenshot");  // Trigger image loading if needed
                    processID = "";  // Reset process ID
                    break;  // Exit the loop after selecting an option
                }
            }
        }


        // Handle submit button
        if (mouseX >= submitButton.x && mouseX <= submitButton.x + submitButton.w &&
            mouseY >= submitButton.y && mouseY <= submitButton.y + submitButton.h) {
            enteringPID = true;
            if (imageTexture) {
                SDL_DestroyTexture(imageTexture);
                imageTexture = nullptr;
            }
            loadImage = false;
            if (selectedOptionText != "None") {
                string numtask = getCurrentTime();
                logMessages.push_back("--------------------------------------------------------------------------------------------------------------");
                logMessages.push_back("* Request sent successfully. Waiting for server response.                        ");
                logMessages.push_back("--------------------------------------------------------------------------------------------------------------");

                scrollPosition = logMessages.size();
                drawRemotePage(renderer, font, sidebar, option1Selected, option2Enabled, option2Title, scrollBar, processID, logMessages, option, selectedOptionText, backgroundTexture, imageTexture, loadImage, visibleLines, logHeight, scrollPosition);
                if (processID != "")
                    newMail(true, selectedOptionText + " \"" + processID + "\"", numtask, "");
                else
                    newMail(true, selectedOptionText, numtask, "");
                processID = "";

                std::string name = "attachment/" + numtask + ".txt";
                //dem++;

                //Wait for file with timeout
                int timeout = 0;
                while (!std::ifstream(name).good() && timeout < 50) {
                    Sleep(1000);
                    cout << "dang doi " << timeout << "\n";
                    timeout++;
                }


                std::ifstream infile(name);

                if (infile.is_open()) {
                    std::string tenfileKem;
                    infile >> tenfileKem;
                    tenfileKem = "attachment/" + tenfileKem;
                    // logMessages.push_back(tenfileKem);



                    if (selectedOptionText == "get_screenshot") {
                        tenfile = tenfileKem;
                        const char* filename = tenfile.c_str();

                        SDL_Surface* loadedImage = IMG_Load(filename);
                        if (loadedImage) {
                            imageTexture = SDL_CreateTextureFromSurface(renderer, loadedImage);
                            SDL_FreeSurface(loadedImage);
                            loadImage = true;
                            logMessages.push_back("Screenshot loaded successfully!");
                        }
                        else {
                            loadImage = false;
                            logMessages.push_back("Failed to load screenshot: " + std::string(IMG_GetError()));
                        }
                    }
                    else {
                        // Read and process attachment file
                        std::ifstream fileKem(tenfileKem);

                        if (fileKem.is_open()) {
                            std::string line;

                            while (std::getline(fileKem, line)) {
                                    logMessages.push_back(line);
                                    // Update scrollPosition to the size of logMessages
                                    scrollPosition = logMessages.size();
                            }
                            fileKem.close();
                        }
                    }
                }
                else{
                    logMessages.push_back("* Server not reponse.");
                    scrollPosition = logMessages.size();
                    drawRemotePage(renderer, font, sidebar, option1Selected, option2Enabled, option2Title, scrollBar, processID, logMessages, option, selectedOptionText, backgroundTexture, imageTexture, loadImage, visibleLines, logHeight, scrollPosition);
                }
            }
        }

        // Handle type request box
        SDL_Rect typeRequestBox = { 130, 500, 180, 50 };
        if (mouseX >= typeRequestBox.x && mouseX <= typeRequestBox.x + typeRequestBox.w &&
            mouseY >= typeRequestBox.y && mouseY <= typeRequestBox.y + typeRequestBox.h) {
            option1Selected = !option1Selected;
        }
    }

}
void handleButtonClick(SDL_Renderer* renderer, SDL_Event& event, int& scrollPosition, int visibleLines, int totalLines, const SDL_Rect& upButton, const SDL_Rect& downButton, const SDL_Rect& clear, std::vector<std::string>& logMessages) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x = event.button.x;
        int y = event.button.y;

        if (x >= upButton.x && x <= upButton.x + upButton.w && y >= upButton.y && y <= upButton.y + upButton.h) {
            cout << " hi";
            scrollPosition = max1(0, scrollPosition - 5); // Move up
        }
        if (x >= downButton.x && x <= downButton.x + downButton.w && y >= downButton.y && y <= downButton.y + downButton.h) {
            scrollPosition = min1(scrollPosition + 5, totalLines - visibleLines); // Move down
        }
        if (x >= clear.x && x <= clear.x + clear.w && y >= clear.y && y <= clear.y + clear.h) {
            logMessages.clear();
            SDL_Rect logConsole = { 120, 70, 650, 390 };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &logConsole);
        }
    }
}
bool validateLogin(const string& username, const string& password) {
    ifstream file("login.txt");
    string fileUsername, filePassword;
    while (file >> fileUsername >> filePassword) {
        if (fileUsername == username && filePassword == password) {
            return true;
        }
    }
    return false;
}
void registerUser(const string& username, const string& password) {
    ofstream file("login.txt", ios::app);
    file << username << " " << password << endl;
    file.close();
}
void handleSidebarAction(int itemIndex, bool& running, bool& loggedIn, SDL_Renderer* renderer, TTF_Font* font) {
    switch (itemIndex) {
    case 0: // Logout
        loggedIn = false;
        currentPage = LOGIN; // Chuyển về trang đăng nhập
        password = "";
        dem = 0;
        break;
    case 1: // Remote
        currentPage = REMOTE; // Đặt trạng thái thành trang Remote
        std::cout << "Navigated to Remote page" << std::endl;
        break;
    case 2: // HDD
        currentPage = HELP;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        std::cout << "Navigated to HDD page" << std::endl;
        break;
    case 3: // Back
        currentPage = APP; // Quay lại trang chính
        break;
    default:
        break;
    }
}
void toggleSidebar(Sidebar& sidebar) {
    sidebar.expanded = !sidebar.expanded;
    sidebar.currentPosition = sidebar.expanded ? EXPANDED_SIDEBAR_WIDTH : INITIAL_SIDEBAR_WIDTH;
}

void copyToClipboard(const string& text) {
    // Mở clipboard
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        size_t size = (text.length() + 1) * sizeof(char); // Tính kích thước bộ nhớ cần cấp phát
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);

        if (hMem) {
            // Copy chuỗi vào bộ nhớ
            memcpy(GlobalLock(hMem), text.c_str(), size);
            GlobalUnlock(hMem);

            // Đặt bộ nhớ vào clipboard
            SetClipboardData(CF_TEXT, hMem);
            GlobalFree(hMem);  // Giải phóng bộ nhớ đã cấp phát
        }
        CloseClipboard();
    }
}

string pasteFromClipboard() {
    string result;
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData) {
            char* text = static_cast<char*>(GlobalLock(hData));
            if (text) {
                result = string(text);
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }
    return result;
}

int runClient(bool &stopClient)
{
    std::string folderName = "attachment"; // Tên thư mục cần xóa
    std::string command = "rmdir /s /q \"" + folderName + "\""; // 

    int result = system(command.c_str()); // Gọi lệnh
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        IMG_Quit();
        return -1;
    }

    if (TTF_Init() == -1) {
        cerr << "TTF_Init Error: " << TTF_GetError() << endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_StartTextInput();  // Bắt đầu chế độ nhập liệu văn bản

    SDL_Window* window = SDL_CreateWindow("Socket", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 17.5);
    TTF_Font* titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 33);

    SDL_Surface* iconSurface = IMG_Load("assets/image1.png");  // Đảm bảo rằng tệp icon.png tồn tại và có định dạng phù hợp
    if (!iconSurface) {
        cerr << "IMG_Load Error: " << IMG_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowIcon(window, iconSurface);  // Gán biểu tượng cho cửa sổ
    SDL_FreeSurface(iconSurface);  // Giải phóng bộ nhớ hình ảnh sau khi gán


    // Load background image
    // với backgroundTexture là của trang login
    // với backgroundTexture1 là của trang app 
    // bổ sung 
    SDL_Surface* backgroundSurface = IMG_Load("assets/background.png");
    SDL_Texture* backgroundTexture = nullptr;
    SDL_Surface* backgroundSurface1 = IMG_Load("assets/background1.png");
    SDL_Texture* backgroundTexture1 = nullptr;
    SDL_Surface* backgroundSurface2 = IMG_Load("assets/background2.png");
    SDL_Texture* backgroundTexture2 = nullptr;
    SDL_Surface* backgroundSurface3 = IMG_Load("assets/background3.png");
    SDL_Texture* backgroundTexture3 = nullptr;

    if (backgroundSurface) {
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);
    }
    if (backgroundSurface1) {
        backgroundTexture1 = SDL_CreateTextureFromSurface(renderer, backgroundSurface1);
        SDL_FreeSurface(backgroundSurface1);
    }
    if (backgroundSurface2) {
        backgroundTexture2 = SDL_CreateTextureFromSurface(renderer, backgroundSurface2);
        SDL_FreeSurface(backgroundSurface2);
    }
    if (backgroundSurface3) {
        backgroundTexture3 = SDL_CreateTextureFromSurface(renderer, backgroundSurface3);
        SDL_FreeSurface(backgroundSurface3);
    }

    Sidebar sidebar = { INITIAL_SIDEBAR_WIDTH, false, false };
    ScrollBar scrollBar = { 0, 20, 200 };
    bool running = true;
    bool loggedIn = false;
    bool registering = false;
    string message;
    bool enteringUsername = true;
    bool option1Selected = false;
    bool option2Enabled = false;
    string option2Title = "None";
    bool enteringPID = true;         // Track Process ID input state
    string processID = "";       // Store Process ID input
    bool draggingSlider = false;  // Biến để theo dõi trạng thái kéo thả thanh trượt
    vector<std::string> logMessages;
    int k = 14;
    vector<int> option(14, 0); // Khởi tạo vector với 6 phần tử, mỗi phần tử có giá trị là 0
    std::string selectedOptionText = "None"; // Default text
    SDL_Texture* imageTexture = nullptr;
    bool loadImage = false;
    SDL_Surface* loadedImage = nullptr;
    bool mail = false;
    string tenfile = "";
    int scrollPosition = 0;
    int logHeight = 20;
    int visibleLines = 390 / logHeight;
    bool quit = false;
    bool check = false;
    bool usernameSelected = false;
    bool passwordSelected = false; 


    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (loggedIn) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Toggle sidebar
                    if (mouseX >= 0 && mouseX <= TOGGLE_BUTTON_SIZE && mouseY >= 0 && mouseY <= TOGGLE_BUTTON_SIZE) {
                        toggleSidebar(sidebar);
                    }

                    // Check if sidebar items are clicked
                    if (sidebar.currentPosition > INITIAL_SIDEBAR_WIDTH && mouseX >= 20 && mouseX <= sidebar.currentPosition) {
                        for (int i = 0; i < (sidebar.showBackButton ? 4 : 3); ++i) {
                            int itemY = 100 + i * 60;
                            if (mouseY >= itemY && mouseY <= itemY + 40) {
                                handleSidebarAction(i, running, loggedIn, renderer, font);
                                if (loggedIn == false) {
                                    processID.clear();  // Xóa ID của quá trình
                                    selectedOptionText = "None";  // Đặt lại lựa chọn
                                    logMessages.clear();  // Xóa danh sách log
                                    enteringPID = true;  // Đặt lại trạng thái nhập PID
                                    loadImage = false;
                                }
                            }
                        }
                    }


                    if (currentPage == REMOTE) {
                        SDL_Rect typeRequestBox = { 130, 500, 180, 50 };
                        SDL_Rect pidBox = { 360, 500, 180, 50 };
                        SDL_Rect submitButton = { 560, 500, 180, 50 };
                        SDL_Rect upButton = { 730, 58, 60, 30 };
                        SDL_Rect downButton = { 660, 58, 70, 30 };
                        SDL_Rect clear = { 590, 58, 70, 30 };

                        // Check if Type of Request dropdown is clicked
                        handleEvents(renderer, event, option1Selected, option, selectedOptionText, loadImage, submitButton, processID, logMessages, tenfile, scrollPosition, logHeight, visibleLines, imageTexture, check, font, sidebar,
                            option2Enabled, option2Title, scrollBar, backgroundTexture1, enteringPID);
                        handleButtonClick(renderer, event, scrollPosition, visibleLines, static_cast<int>(logMessages.size()), upButton, downButton, clear, logMessages);
                        
                        // Check if Process ID box is clicked
                        if (mouseX >= pidBox.x && mouseX <= pidBox.x + pidBox.w &&
                            mouseY >= pidBox.y && mouseY <= pidBox.y + pidBox.h) {
                            if (mouseX >= pidBox.x && mouseX <= pidBox.x + pidBox.w &&
                                mouseY >= pidBox.y && mouseY <= pidBox.y + pidBox.h) {
                                if (selectedOptionText == "get_file" || selectedOptionText == "close_app" || selectedOptionText == "run_app" || selectedOptionText == "list_files" || selectedOptionText == "close_by_id"|| selectedOptionText == "servers_IP" || selectedOptionText == "delete_file"|| selectedOptionText == "change_server")
                                {
                                    enteringPID = true;
                                }
                            }
                            else {

                                enteringPID = true;

                            }
                        }

                        if (selectedOptionText != "None" && selectedOptionText != "get_screenshot") {
                            if (mouseX >= submitButton.x && mouseX <= submitButton.x + submitButton.w &&
                                mouseY >= submitButton.y && mouseY <= submitButton.y + submitButton.h) {

                                mail = true;




                                // Remove oldest log if logMessages exceeds display limit
                                if (logMessages.size() > 10) {
                                    logMessages.erase(logMessages.begin());
                                }
                            }
                        }
                        else if (selectedOptionText == "get_screenshot" && loadImage != false) {
                            logMessages.clear();
                            if (mouseX >= submitButton.x && mouseX <= submitButton.x + submitButton.w &&
                                mouseY >= submitButton.y && mouseY <= submitButton.y + submitButton.h) {

                            }
                        }

                    }
                }
                if (event.type == SDL_TEXTINPUT && currentPage == REMOTE) {
                    if (enteringPID && processID.length() < 50  /* MAX_TEXT_LENGTH */) {
                        processID += event.text.text;
                    }
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.mod & KMOD_CTRL) {
                    if (event.key.keysym.sym == SDLK_c && enteringPID) {
                        // Copy nội dung của processID vào clipboard
                        copyToClipboard(processID);
                    }

                    // Kiểm tra khi người dùng nhấn Ctrl + V (paste)
                    if (event.key.keysym.sym == SDLK_v && enteringPID) {
                        // Dán nội dung từ clipboard vào processID
                        processID = pasteFromClipboard();
                    }


                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE && currentPage == REMOTE) {
                    // Kiểm tra xem người dùng có đang nhập PID và PID không rỗng
                    if (enteringPID && !processID.empty()) {
                        // Xóa ký tự cuối cùng trong processID
                        processID.pop_back();
                    }
                }

            }

            else {
                // Xử lý các sự kiện trên trang LOGIN
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
                    passwordSelected = false;
                    usernameSelected = false;
                    // Handle the "Login/Register" button click
                    if (mouseX >= 70 && mouseX <= 340 && mouseY >= 380 && mouseY <= 420) {
                        if (registering) {
                            registerUser(username, password);
                            message = "Registration successful!";
                            username.clear();
                            password.clear();
                            registering = false; // Switch to login mode after registration
                        }
                        else if (validateLogin(username, password)) {
                            loggedIn = true; // Successful login
                            currentPage = APP; // Navigate to the app page
                            message.clear();
                        }
                        else {
                            message = "Invalid username or password."; // Show error
                        }
                    }
                    // Handle the "Register" button click when in login mode
                    else if (!registering && mouseX >= 70 && mouseX <= 340 && mouseY >= 430 && mouseY <= 470) {
                        registering = true; // Switch to registration mode
                        username.clear();
                        password.clear();
                        message.clear();
                    }
                    // Handle username input box click
                    else if (mouseX >= 170 && mouseX <= 380 && mouseY >= 250 && mouseY <= 280) {
                        enteringUsername = true;
                        usernameSelected = true;
                        passwordSelected = false;
                    }
                    // Handle password input box click
                    else if (mouseX >= 170 && mouseX <= 380 && mouseY >= 300 && mouseY <= 330) {
                        enteringUsername = false;
                        usernameSelected = false; 
                        passwordSelected = true; 
                    }
                    
                }

                if (event.type == SDL_TEXTINPUT) {
                    // Append text input to username or password
                    if (enteringUsername && username.length() < MAX_TEXT_LENGTH) {
                        username += event.text.text;
                    }
                    else if (!enteringUsername && password.length() < MAX_TEXT_LENGTH) {
                        password += event.text.text;
                    }
                }

                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE) {
                    // Handle backspace for username and password
                    if (enteringUsername && !username.empty()) {
                        username.pop_back();
                    }
                    else if (!enteringUsername && !password.empty()) {
                        password.pop_back();
                    }
                }
            }

            // Render appropriate page based on currentPage
            if (currentPage == LOGIN) {
                drawLoginPage(renderer, font, titleFont, username, password, registering, message, backgroundTexture, usernameSelected, passwordSelected);
            }
            else if (currentPage == APP) {
                drawAppPage(renderer, font, sidebar, backgroundTexture2);
            }
            else if (currentPage == REMOTE) {
                drawRemotePage(renderer, font, sidebar, option1Selected, option2Enabled, option2Title, scrollBar, processID, logMessages, option, selectedOptionText, backgroundTexture1, imageTexture, loadImage, visibleLines, logHeight, scrollPosition);
            }
            else if (currentPage == HELP) {
                drawHelpPage(renderer, font, sidebar, backgroundTexture3);
            }

            SDL_RenderPresent(renderer);

        }

    }


    SDL_StopTextInput();  // Kết thúc chế độ nhập liệu văn bản
    TTF_CloseFont(titleFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (imageTexture) SDL_DestroyTexture(imageTexture);
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    TTF_Quit();
    SDL_Quit;
    stopClient = true;
    return 0;
}

void runListen(bool &stopClient)
{
    autoGetMail(stopClient, true);
    return;
}

int main(int argc, char* argv[])
{
    bool stopClient = false;
    vector<thread> threads;
    threads.emplace_back(ref(runClient), ref(stopClient));
    threads.emplace_back(ref(runListen), ref(stopClient));
    
    for(auto &thread: threads)
        thread.join();
    return 0;
}