#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>

// Map for base64
std::map<char, int> base64_map = {
    {'A', 0}, {'B', 1}, {'C', 2}, {'D', 3}, {'E', 4}, {'F', 5},
    {'G', 6}, {'H', 7}, {'I', 8}, {'J', 9}, {'K', 10}, {'L', 11},
    {'M', 12}, {'N', 13}, {'O', 14}, {'P', 15}, {'Q', 16}, {'R', 17},
    {'S', 18}, {'T', 19}, {'U', 20}, {'V', 21}, {'W', 22}, {'X', 23},
    {'Y', 24}, {'Z', 25}, {'a', 26}, {'b', 27}, {'c', 28}, {'d', 29},
    {'e', 30}, {'f', 31}, {'g', 32}, {'h', 33}, {'i', 34}, {'j', 35},
    {'k', 36}, {'l', 37}, {'m', 38}, {'n', 39}, {'o', 40}, {'p', 41},
    {'q', 42}, {'r', 43}, {'s', 44}, {'t', 45}, {'u', 46}, {'v', 47},
    {'w', 48}, {'x', 49}, {'y', 50}, {'z', 51}, {'0', 52}, {'1', 53},
    {'2', 54}, {'3', 55}, {'4', 56}, {'5', 57}, {'6', 58}, {'7', 59},
    {'8', 60}, {'9', 61}, {'+', 62}, {'/', 63}, {'=', -1}
};

// Ham giai ma base64
std::vector<unsigned char> base64_decode(const std::string &in) {
    std::vector<unsigned char> out;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (base64_map.find(c) == base64_map.end()) continue; // Skip ki tu ko phai base64
        val = (val << 6) + base64_map[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}

// Ham doc ma base64
std::string read_base64_from_file(const std::string &filename) {
    std::ifstream infile(filename);
    std::string base64_data, line;

    while (std::getline(infile, line)) {
        base64_data += line; // Kết hợp các dòng
    }

    return base64_data;
}

void saveFile(const std::string &filename, const std::vector<unsigned char> &data) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
}

int main() {
    // Doc ma base64
    std::string base64_filename = "latest_email.eml"; // Tệp chứa mã base64
    std::string base64_data = read_base64_from_file(base64_filename);

    // Giai ma base64
    std::vector<unsigned char> decoded_data = base64_decode(base64_data);

    // Luu file
    saveFile("note.txt", decoded_data);

    std::cout << "Gia ma xong" << std::endl;
    system("pause");
    return 0;
}



// #include <cstdio>
// #include <cstdlib>
// #include <iostream>
// #include <fstream>
// #include <string>
// #include <vector>

// using namespace std;

// void processEML(string emlPath){
//     ifstream input(emlPath);
//     string line;
//     int i = 0;
//     while(getline(input, line)){
//         cout << "- "<< line << "\n";
//         i++; if (i == 20) break;
//     }
// }

// int main() {
//     string emlPath = "latest_email.eml";
//     processEML(emlPath);
//     system("pause");
//     return 0;
// }