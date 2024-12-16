#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "socket.h"
#include <algorithm>
#define NOMINMAX  
#include <windows.h>
#include <filesystem>
#include <cstdlib> 
#include<thread>
#include <windows.h>
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
using namespace std;

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

    const char* sidebarItems[] = { "Logout", "Remote", "Help"};
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
    SDL_Rect line1 = { 5, 2, lineWidth+20, lineHeight };  // Dấu ngạch dậm đầu tiên
    SDL_Rect line2 = { 5, 2 + lineHeight+1.5 + spacing, lineWidth+20, lineHeight };  // Dấu ngạch dậm thứ hai
    SDL_Rect line3 = { 5, 2 + 2 * (lineHeight + spacing)+3, lineWidth+20, lineHeight };  // Dấu ngạch dậm thứ ba

    // Vẽ các dấu ngạch dậm
    SDL_RenderFillRect(renderer, &line1);
    SDL_RenderFillRect(renderer, &line2);
    SDL_RenderFillRect(renderer, &line3);
}

// vẽ trang login 
void drawLoginPage(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* titleFont, const string& username, const string& password, bool registering, const string& message, SDL_Texture* backgroundTexture) {
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 215, 0);
    //SDL_Rect borderRect = { 37, 180, 360, 330 };
    //SDL_RenderDrawRect(renderer, &borderRect);

    SDL_Color textColor = { 255, 215, 0 };
    renderText(renderer, titleFont, registering ? "REGISTER" : "  LOGIN", 158, 180, textColor);
    renderText(renderer, font, "USERNAME:", 70, 250, textColor);

    SDL_Rect usernameBox = { 60, 248, 280, 35 };
    SDL_RenderDrawRect(renderer, &usernameBox);

    // Render văn bản người dùng đã nhập
    renderText(renderer, font, username, 175, 250, textColor, usernameBox.w - 10);

    // Lấy thời gian hiện tại để thay đổi trạng thái con trỏ mỗi 500ms
    Uint32 currentTime = SDL_GetTicks();
    if ((currentTime / 500) % 2 == 0) {  // Đổi trạng thái con trỏ mỗi 500ms
        // Tính toán vị trí của con trỏ dựa trên độ dài chuỗi username
        int cursorX = 175 + username.length() * 10;  // Giả định rằng mỗi ký tự chiếm 10px

        // Tạo hiệu ứng nhấp nháy cho con trỏ nhỏ
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Màu trắng cho con trỏ
        int cursorHeight = 20; // Chiều cao con trỏ nhỏ hơn (ví dụ: 20px)
        SDL_RenderDrawLine(renderer, cursorX, 249, cursorX, 249 + cursorHeight);  // Vẽ con trỏ tại vị trí
    }

    renderText(renderer, font, "PASSWORD:", 65, 300, textColor);
    SDL_Rect passwordBox = { 60, 298, 280, 35 };
    SDL_RenderDrawRect(renderer, &passwordBox);

    // Tạo mật khẩu ẩn bằng dấu sao (hiddenPassword)
    string hiddenPassword(password.size(), '*');

    // Render mật khẩu ẩn vào ô nhập mật khẩu
    renderText(renderer, font, hiddenPassword, 175, 300, textColor, passwordBox.w - 10);

    // Lấy thời gian hiện tại để thay đổi trạng thái con trỏ mỗi 500ms
   // Đảm bảo lấy thời gian hiện tại
    if ((currentTime / 500) % 2 == 0) {  // Đổi trạng thái con trỏ mỗi 500ms
        // Tính toán vị trí của con trỏ dựa trên độ dài chuỗi password
        int cursorX = 175 + password.length() * 10;  // Giả định rằng mỗi ký tự chiếm 10px

        // Tạo hiệu ứng nhấp nháy cho con trỏ nhỏ
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Màu trắng cho con trỏ
        int cursorHeight = 20;  // Chiều cao con trỏ
        SDL_RenderDrawLine(renderer, cursorX, 299, cursorX, 299 + cursorHeight);  // Vẽ con trỏ tại vị trí
    }

    SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255);
    SDL_Rect actionButton = { 70, 380, 280, 40 };
    SDL_RenderFillRect(renderer, &actionButton);
    renderText(renderer, font, registering ? "  REGISTER" : "  LOGIN", 160, 385, textColor);

    if (!registering) {
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_Rect registerButton = { 70, 430, 280, 40 };
        SDL_RenderFillRect(renderer, &registerButton);
        renderText(renderer, font, "REGISTER", 145, 435, textColor);
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

    sidebar.showBackButton = false;
    renderSidebar(renderer, sidebar, font);

    SDL_Color textColor = { 255, 255, 255 };
    int textOffset = sidebar.currentPosition;
    renderText(renderer, font, "HELP", 400, 70, textColor);

    renderText(renderer, font, "List apps: list_apps", 130 , 100, textColor);
    renderText(renderer, font, "List services: list_services", 130 , 130, textColor);
    renderText(renderer, font, "Take screenshot: get_screenshot", 130, 160, textColor);
    renderText(renderer, font, "Shutdown: shutdown", 130, 190, textColor);
    renderText(renderer, font, "Turn on/off webcam: webcam_on / webcam_off", 130, 220, textColor);
    renderText(renderer, font, "Get file: get_file ", 130, 250, textColor);
    renderText(renderer, font, "List files: list_files", 130, 280, textColor);
    renderText(renderer, font, "Find app path: find_path", 130, 310, textColor);
    renderText(renderer, font, "Run app: run_app", 130, 340, textColor);
    renderText(renderer, font, "Close app: close_app", 130, 370, textColor);
    renderText(renderer, font, "View running apps: running_apps", 130, 400, textColor);
 



    


}
void drawSubmitButton(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Rect submitButton = { 560, 500, 180, 50 };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // màu đỏ
    SDL_RenderFillRect(renderer, &submitButton);
    renderText(renderer, font, "SUBMIT", 620, 520, { 255, 255, 255 });
}
void drawButton(SDL_Renderer* renderer, const SDL_Rect& rect, const std::string&  text, TTF_Font* font, SDL_Color color, SDL_Color textColor) {
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
    bool loadImage, int visibleLines, int logHeight, int& scrollPosition ) {

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
            scrollPosition =max1(0, min1(scrollPosition, totalLines - visibleLines));

            // Render visible lines
            for (int i = scrollPosition; i < min1(scrollPosition + visibleLines, totalLines); ++i) {
                int textY = logConsole.y + (i - scrollPosition) * logHeight; // Adjust Y position for each line
                renderText(renderer, font, logMessages[i], logConsole.x + 5, textY, { 0, 255, 0 });
            }
        }

        
    }

    SDL_Rect upButton = { 730, 58, 60, 30 };
    SDL_Rect downButton = { 660, 58, 70, 30 };
    SDL_Rect clear = { 590, 58, 70, 30 };

    drawButton(renderer, upButton, "Up", font, { 100, 100, 255, 255 }, { 255, 255, 255, 255 });
    drawButton(renderer, downButton, "Down", font, { 100, 100, 255, 255 }, { 255, 255, 255, 255 });
    drawButton(renderer, clear, " Clear", font, { 100, 100, 255, 255 }, { 255, 255, 255, 255 });


    // Type of Request Dropdown
    SDL_Rect typeRequestBox = { 130, 500, 180, 50 };
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_RenderFillRect(renderer, &typeRequestBox);
    renderText(renderer, font, "Option", 150, 520, { 255, 255, 255 });
    renderText(renderer, font, "Select option ", 160, 470, { 255, 255, 0 });

    // Process ID Box
    SDL_Rect pidBox = { 350, 500, 180, 50 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &pidBox);
   
    SDL_Color textColor = { 255, 255, 255 };

    // Xử lý đoạn văn bản dài hơn 20 hoặc 30 ký tự
    std::string displayText = processID;  // Đoạn văn bản cần hiển thị

    if (processID.length() > 30) {
        displayText = processID.substr(processID.length() - 13);  // Lấy 10 ký tự cuối
    }
    else if (processID.length() > 20) {
        displayText = processID.substr(processID.length() - 14);  // Lấy 10 ký tự cuối
    }

    // Render đoạn văn bản đã được cắt (hiển thị 10 ký tự cuối cùng)
    renderText(renderer, font, displayText, 360, 520, textColor);

 

    // Hiển thị con trỏ nhập liệu (dấu nháy) để người dùng biết vị trí
    //Uint32 currentTime = SDL_GetTicks();
    //if ((currentTime / 500) % 2 == 0) {  // Đổi trạng thái con trỏ mỗi 500ms
        //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Màu trắng cho con trỏ
        //SDL_RenderDrawLine(renderer, 307 + processID.length() * 10, 523, 307 + processID.length() * 10, 523 + 30);  // Dựng con trỏ
    //}

    // Dropdown menu
    if (option1Selected) {
        SDL_Rect dropdownBox = { 218, 120, 465 , 230 };
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(renderer, &dropdownBox);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &dropdownBox);

        const std::vector<std::string> optionTexts = {
            "List app", "List services", "Shutdown",
            "Webcam on", "Webcam off", "Screenshot" ,
            "Get File" , "List File" , 
            "Run App" , "Close App", "App Run"
        };

        const int OPTION_WIDTH = 220;  // Chiều rộng của mỗi hộp
        const int OPTION_HEIGHT = 35;  // Chiều cao của mỗi hộp
        const int MARGIN_X_LEFT = 222; // Vị trí X của cột trái
        const int MARGIN_X_RIGHT = 452; // Vị trí X của cột phải
        const int MARGIN_Y = 123;  // Vị trí Y ban đầu
        const int SPACING_Y = 38;  // Khoảng cách giữa các tùy chọn

        for (size_t i = 0; i < optionTexts.size(); ++i) {
            // Tính toán vị trí X cho cột trái và cột phải
            int posX = (i < 6) ? MARGIN_X_LEFT : MARGIN_X_RIGHT;

            // Tính toán vị trí Y cho các tùy chọn (sắp xếp theo chiều dọc)
            int posY = MARGIN_Y + static_cast<int>(i % 6) * SPACING_Y;

            // Vẽ hộp tùy chọn
            SDL_Rect optionBox = { posX, posY, OPTION_WIDTH, OPTION_HEIGHT };
            SDL_SetRenderDrawColor(renderer, option[i] == 1 ? 0 : 128, 128, 128, 255);  // Màu nền
            SDL_RenderFillRect(renderer, &optionBox);  // Vẽ nền
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Màu viền trắng
            SDL_RenderDrawRect(renderer, &optionBox);  // Vẽ viền

            // Vẽ văn bản cho mỗi tùy chọn
            renderText(renderer, font, optionTexts[i],
                posX + 5,  // Vị trí X của văn bản (căn trái với khoảng cách)
                posY + 3,  // Vị trí Y của văn bản (căn giữa với hộp)
                { 255, 255, 255 });  // Màu chữ trắng
        }
    }

    // Selected option display
    renderText(renderer, font, "Selected: " + selectedOptionText,
        375, 470, { 255, 255, 0 });

    drawSubmitButton(renderer, font);
    SDL_RenderPresent(renderer);
}

void handleEvents(SDL_Renderer* renderer, SDL_Event& event, bool& option1Selected,
    std::vector<int>& option, std::string& selectedOptionText,
    bool& loadImage, SDL_Rect& submitButton,
     std::string& processID, std::vector<std::string>& logMessages,
    std::string& tenfile, int& scrollPosition,
    int logHeight, int visibleLines, SDL_Texture*& imageTexture ,
    bool& check, TTF_Font* font, Sidebar& sidebar,
     bool option2Enabled, const std::string& option2Title, ScrollBar& scrollBar, SDL_Texture* backgroundTexture) {

    static int dem = 0; // Message counter

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;

        // Handle dropdown options
        if (option1Selected) {
            const std::vector<std::string> optionTexts = {
                "list_apps", "list_services", "shutdown",
                "webcam_on", "webcam_off", "get_screenshot" , 
                "get_file" , "list_files"  , "run_apps" ,"close_app" , "running_app" 
            };

            const int OPTION_WIDTH = 220;  // Chiều rộng của mỗi hộp
            const int OPTION_HEIGHT = 35;  // Chiều cao của mỗi hộp
            const int MARGIN_X_LEFT = 222; // Vị trí X của cột trái
            const int MARGIN_X_RIGHT = 452; // Vị trí X của cột phải
            const int MARGIN_Y = 123;  // Vị trí Y ban đầu
            const int SPACING_Y = 38;  // Khoảng cách giữa các tùy chọn

            for (size_t i = 0; i < optionTexts.size(); ++i) {
                // Tính toán vị trí X cho cột trái và cột phải
                int posX = (i < 6) ? MARGIN_X_LEFT : MARGIN_X_RIGHT;

                // Tính toán vị trí Y cho các tùy chọn (sắp xếp theo chiều dọc)
                int posY = MARGIN_Y + static_cast<int>(i % 6) * SPACING_Y;

                // Vẽ hộp tùy chọn
                SDL_Rect optionBox = { posX, posY, OPTION_WIDTH, OPTION_HEIGHT };

                if (mouseX >= optionBox.x && mouseX <= optionBox.x + optionBox.w &&
                    mouseY >= optionBox.y && mouseY <= optionBox.y + optionBox.h) {

                    std::fill(option.begin(), option.end(), 0);
                    option[i] = 1;
                    selectedOptionText = optionTexts[i];
                    option1Selected = false;
                    loadImage = (optionTexts[i] == "get_screenshot");
                    processID = ""; 
                    break;
                }

            }
        }

        // Handle submit button
        if (mouseX >= submitButton.x && mouseX <= submitButton.x + submitButton.w &&
            mouseY >= submitButton.y && mouseY <= submitButton.y + submitButton.h) {

            if (imageTexture) {
                SDL_DestroyTexture(imageTexture);
                imageTexture = nullptr;
            }
            loadImage = false;
            if (selectedOptionText != "None") {
                
                if (processID != "")
                    newMail(true, selectedOptionText + " \"" + processID + "\"", int_to_string(dem), "");
                else
                    newMail(true, selectedOptionText, int_to_string(dem), "");
                processID = "";

                logMessages.push_back("ID: " + int_to_string(dem) + " - Mail sent successfully. Waiting for a response sever.");


                drawRemotePage(renderer, font, sidebar, option1Selected, option2Enabled, option2Title, scrollBar, processID, logMessages, option, selectedOptionText, backgroundTexture, imageTexture, loadImage, visibleLines, logHeight, scrollPosition);



                std::string name = "attachment/" + int_to_string(dem) + ".txt";
                dem++;

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
                                std::istringstream stream(line); // Use stringstream to tokenize the line
                                std::string word;
                                std::string chunk;
                                size_t totalLength = 0;

                                // Iterate through each word in the line
                                while (stream >> word) {
                                    // Check if adding the word exceeds the 70-character limit
                                    if (totalLength + word.size() + (totalLength > 0 ? 1 : 0) > 70) {
                                        if (!chunk.empty()) {
                                            logMessages.push_back(chunk); // Save the current chunk
                                        }
                                        chunk = word;                  // Start a new chunk with the current word
                                        totalLength = word.size();     // Reset total length
                                    }
                                    else {
                                        // Add the word to the current chunk
                                        if (!chunk.empty()) {
                                            chunk += " "; // Add a space between words
                                        }
                                        chunk += word;
                                        totalLength += word.size() + 1; // Update the total length (including space)
                                    }
                                }

                                // Add any remaining chunk to logMessages
                                if (!chunk.empty()) {
                                    logMessages.push_back(chunk);
                                    scrollPosition = logMessages.size();
                                }
                            }

                            // Update scrollPosition to the size of logMessages
               

                            fileKem.close();
                        }
                    }
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
void runBatFile(const std::string& batFilePath) {
    int result = system(batFilePath.c_str());
    if (result == 0) {
        std::cout << "Tệp .bat đã được chạy thành công!" << std::endl;
    }
    else {
        std::cerr << "Lỗi khi chạy tệp .bat." << std::endl;
    }
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
int main(int argc, char* argv[]) {
    std::string folderName = "attachment"; // Tên thư mục cần xóa
    std::string command = "rmdir /s /q \"" + folderName + "\""; // 
    std::thread batThread(runBatFile, "runLISTEN");
    

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

    SDL_Surface* iconSurface = IMG_Load("image1.png");  // Đảm bảo rằng tệp icon.png tồn tại và có định dạng phù hợp
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
    SDL_Surface* backgroundSurface = IMG_Load("background.png");
    SDL_Texture* backgroundTexture = nullptr;
    SDL_Surface* backgroundSurface1 = IMG_Load("background1.png");
    SDL_Texture* backgroundTexture1 = nullptr;
    SDL_Surface* backgroundSurface2 = IMG_Load("background2.png");
    SDL_Texture* backgroundTexture2 = nullptr;
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
    bool enteringPID = false;         // Track Process ID input state
    string processID = "";       // Store Process ID input
    bool draggingSlider = false;  // Biến để theo dõi trạng thái kéo thả thanh trượt
    vector<std::string> logMessages;
    int k = 12;  
    vector<int> option(12, 0); // Khởi tạo vector với 6 phần tử, mỗi phần tử có giá trị là 0
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
                                    enteringPID = false;  // Đặt lại trạng thái nhập PID
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
                        handleEvents(renderer, event, option1Selected, option, selectedOptionText, loadImage, submitButton, processID, logMessages, tenfile, scrollPosition, logHeight, visibleLines,imageTexture,check , font, sidebar,
                            option2Enabled, option2Title, scrollBar,  backgroundTexture1);
                        handleButtonClick(renderer,event, scrollPosition, visibleLines, static_cast<int>(logMessages.size()), upButton, downButton, clear, logMessages);
                        enteringPID = false;
                        // Check if Process ID box is clicked
                        if (mouseX >= pidBox.x && mouseX <= pidBox.x + pidBox.w &&
                            mouseY >= pidBox.y && mouseY <= pidBox.y + pidBox.h) {
                            if (mouseX >= pidBox.x && mouseX <= pidBox.x + pidBox.w &&
                                mouseY >= pidBox.y && mouseY <= pidBox.y + pidBox.h) {
                                if (selectedOptionText == "get_file" || selectedOptionText == "find_path" || selectedOptionText == "close_app"  || selectedOptionText == "run_apps" || selectedOptionText == "list_files")
                                {
                                    enteringPID = true;
                                }
                            }
                            else {
                                
                                enteringPID = false;

                            }
                        }

                        if (selectedOptionText != "None"  && selectedOptionText != "get_screenshot") {
                            if (mouseX >= submitButton.x && mouseX <= submitButton.x + submitButton.w &&
                                mouseY >= submitButton.y && mouseY <= submitButton.y + submitButton.h) {

                                mail = true; 
                                
                                

                                // Remove oldest log if logMessages exceeds display limit
                                if (logMessages.size() > 10) {
                                    logMessages.erase(logMessages.begin());
                                }
                            }
                        }
                        else if (selectedOptionText == "get_screenshot"  && loadImage != false) {
                            logMessages.clear();
                            if (mouseX >= submitButton.x && mouseX <= submitButton.x + submitButton.w &&
                                mouseY >= submitButton.y && mouseY <= submitButton.y + submitButton.h) {

                            }
                        }

                    }
                }
                if (event.type == SDL_TEXTINPUT && currentPage == REMOTE ) {
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
                    }
                    // Handle password input box click
                    else if (mouseX >= 170 && mouseX <= 380 && mouseY >= 300 && mouseY <= 330) {
                        enteringUsername = false;
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
                drawLoginPage(renderer, font, titleFont, username, password, registering, message, backgroundTexture);
            }
            else if (currentPage == APP) {
                drawAppPage(renderer, font, sidebar, backgroundTexture2);
            }
            else if (currentPage == REMOTE) {
                drawRemotePage(renderer, font, sidebar, option1Selected, option2Enabled, option2Title, scrollBar, processID, logMessages, option, selectedOptionText, backgroundTexture1, imageTexture, loadImage, visibleLines, logHeight, scrollPosition);
            }
            else if (currentPage == HELP) {
                drawHelpPage(renderer, font, sidebar, backgroundTexture1);
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
    batThread.join();
    TTF_Quit();
    SDL_Quit;;
    return 0;
}
