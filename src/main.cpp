#include "json.hpp"
using json = nlohmann::json;

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

// Include standard headers first
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <map>
#include <mutex>
#include <random>
#include <cmath>
#include <limits>

// Then Windows headers
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <wtypes.h>
#include <psapi.h>
#include <wininet.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")



std::string get_log_path();
void ensure_log_directory_exists();

std::string get_exe_path();

std::mutex console_mutex;

// Original resolution for which coordinates were calibrated
std::pair<int, int> RES_ORIG = { 1920, 1200 };

// Function declaration for log_message
void log_message(const std::string& message);

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Safe time conversion function
std::tm localtime_xp(std::time_t timer) {
    std::tm bt{};
#if defined(__unix__)
    localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
    localtime_s(&bt, &timer);
#else
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    bt = *std::localtime(&timer);
#endif
    return bt;
}

// Log to file (everything)
void log_to_file(const std::string& message) {
    try {
        std::string log_path = get_exe_path() + "automation_log.txt";

        // Try to open the file
        std::ofstream logfile(log_path, std::ios_base::app);
        if (logfile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto now_time = std::chrono::system_clock::to_time_t(now);
            std::tm bt = localtime_xp(now_time);

            logfile << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
            logfile.close();
        }
        else {
            // If it fails, try to create the directory and try again
            std::string dir = get_exe_path();
            if (!PathIsDirectoryA(dir.c_str())) {
                CreateDirectoryA(dir.c_str(), NULL);

                std::ofstream new_log(log_path, std::ios_base::app);
                if (new_log.is_open()) {
                    auto now = std::chrono::system_clock::now();
                    auto now_time = std::chrono::system_clock::to_time_t(now);
                    std::tm bt = localtime_xp(now_time);

                    new_log << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
                    new_log.close();
                    return;
                }
            }

            // If it still fails, show detailed error
            DWORD err = GetLastError();
            LPSTR err_msg = nullptr;
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err, 0, (LPSTR)&err_msg, 0, NULL);

            std::cerr << "Failed to open log file! Path: " << log_path
                << " Error: " << (err_msg ? err_msg : "Unknown")
                << " Message: " << message << std::endl;

            if (err_msg) LocalFree(err_msg);
        }
    }
    catch (...) {
        std::cerr << "Unexpected error while logging" << std::endl;
    }
}

// Show progress in console + log to file
void show_progress(const std::string& message) {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << message << std::endl;
    log_to_file("[PROGRESS] " + message);
}

// Error log (red console + file)
void log_error(const std::string& message) {
    std::lock_guard<std::mutex> lock(console_mutex);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 12);
    std::cerr << "[ERROR] " << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
    log_to_file("[ERROR] " + message);
}

// Loading log (file only)
void log_load(const std::string& message) {
    log_to_file("[LOADING] " + message);
}

// Structure for coordinates
struct Coordinate {
    int x;
    int y;
    std::string description;
};

std::string wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }

    // Determine required size for converted string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed == 0) {
        log_error("Error determining size for wstring to string conversion");
        return std::string();
    }

    // Allocate buffer for converted string
    std::string str(size_needed, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    if (result == 0) {
        log_error("Error converting wstring to string");
        return std::string();
    }

    return str;
}

// Global variable to store coordinates (NOW DECLARED BEFORE FUNCTIONS THAT USE IT)
std::map<std::string, Coordinate> coordinates;
bool resolution_loaded = false;
std::pair<int, int> saved_resolution = { 0, 0 };

// Function prototypes
void pause();
std::pair<int, int> get_current_resolution();
std::pair<int, int> adjust_coordinates(int x, int y);
void check_configure_resolution();
void header();
void save_resolution_json();
std::string get_exe_path();

void donate_hearts();
void donate_hearts_2();
void donate_hearts_3();
void donate_hearts_4();
void donate_hearts_5();
void donate_hearts_6();
void donate_hearts_7();
void donate_hearts_8();
void donate_hearts_9();
void donate_hearts_10();

// Get current resolution
std::pair<int, int> get_current_resolution() {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    return { x, y };
}

// Complete adjust_coordinates function
std::pair<int, int> adjust_coordinates(int x, int y) {
    auto current_res = get_current_resolution();

    // Calculate individual ratios
    float ratio_x = static_cast<float>(current_res.first) / RES_ORIG.first;
    float ratio_y = static_cast<float>(current_res.second) / RES_ORIG.second;

    // Use the smallest ratio to maintain proportion (avoid distortion)
    float ratio = std::min(ratio_x, ratio_y);

    // Calculate margins to center
    int offset_x = static_cast<int>((current_res.first - (RES_ORIG.first * ratio)) / 2);
    int offset_y = static_cast<int>((current_res.second - (RES_ORIG.second * ratio)) / 2);

    // Apply adjustment
    int adjusted_x = static_cast<int>(x * ratio) + offset_x;
    int adjusted_y = static_cast<int>(y * ratio) + offset_y;

    return { adjusted_x, adjusted_y };
}

void debug_coordinates(const std::string& point_name) {
    if (coordinates.find(point_name) != coordinates.end()) {
        auto& coord = coordinates[point_name];
        auto adjusted = adjust_coordinates(coord.x, coord.y);

        log_load("Debug coordinate '" + point_name + "':");
        log_load("Original: (" + std::to_string(coord.x) + ", " + std::to_string(coord.y) + ")");
        log_load("Adjusted: (" + std::to_string(adjusted.first) + ", " + std::to_string(adjusted.second) + ")");

        // Move mouse to adjusted position (for visualization)
        SetCursorPos(adjusted.first, adjusted.second);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// Complete pause function
void pause() {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << "\nPress Enter to continue...";
    std::cout.flush(); // Ensure message is displayed immediately
    std::cin.clear(); // Clear std::cin error state
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void check_configure_resolution() {
    header();
    auto current_res = get_current_resolution();
    std::cout << " RESOLUTION CONFIGURATION\n";
    std::cout << "=============================================\n";
    std::cout << " Detected resolution: " << current_res.first << "x" << current_res.second << "\n";
    std::cout << " Current reference resolution: " << RES_ORIG.first << "x" << RES_ORIG.second << "\n";
    // Show saved resolution
    if (resolution_loaded && saved_resolution.first > 0 && saved_resolution.second > 0) {
        std::cout << " Saved resolution: " << saved_resolution.first << "x" << saved_resolution.second << "\n";
    }
    else {
        std::cout << " Saved resolution: No saved resolution found\n";
    }
    std::cout << "\n";

    std::cout << "1. Use detected resolution (" << current_res.first << "x" << current_res.second << ")\n";
    std::cout << "2. Enter resolution manually\n";
    std::cout << "3. Keep default resolution (" << RES_ORIG.first << "x" << RES_ORIG.second << ")\n";
    std::cout << "0. Back\n\n";
    std::cout << "Choose an option: ";

    int choice;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Invalid option!\n";
        pause();
        return;
    }

    switch (choice) {
    case 1: {
        const_cast<std::pair<int, int>&>(RES_ORIG) = current_res;
        std::cout << "\nResolution set to: " << current_res.first << "x" << current_res.second << "\n";
        std::cout << "All coordinates will be adjusted automatically.\n";
        save_resolution_json();
        break;
    }
    case 2: {
        int width, height;
        std::cout << "\nEnter width: ";
        while (!(std::cin >> width) || width < 800 || width > 7680) { // Example: min 800, max 8K
            std::cout << "Invalid width! Enter a number between 800 and 7680: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cout << "Enter height: ";
        while (!(std::cin >> height) || height < 600 || height > 4320) { // Example: min 600, max 8K
            std::cout << "Invalid height! Enter a number between 600 and 4320: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        const_cast<std::pair<int, int>&>(RES_ORIG) = { width, height };
        std::cout << "\nResolution set to: " << width << "x" << height << "\n";
        save_resolution_json();
        break;
    }
    case 3: {
        std::cout << "\nKeeping default resolution: " << RES_ORIG.first << "x" << RES_ORIG.second << "\n";
        save_resolution_json();
        break;
    }
    case 0:
        return;
    default:
        std::cout << "Invalid option!\n";
        break;
    }

    pause();
}

// Function to get executable path
std::string get_exe_path() {
    char path[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        log_error("Error getting executable path");
        return "";
    }

    if (!PathRemoveFileSpecA(path)) {
        log_error("Error removing filename from path");
        return "";
    }

    std::string result(path);
    if (!result.empty() && result.back() != '\\') {
        result += "\\";
    }

    return result;
}

// Function to load coordinates from JSON
void load_coordinates() {
    std::string full_path = get_exe_path() + "coordinates.json";
    std::ifstream file(full_path);

    if (!file.is_open()) {
        log_error("Error opening coordinates.json file at path: " + full_path);
        return;
    }

    try {
        json data;
        file >> data;

        // Load saved resolution if it exists
        if (data.contains("saved_resolution")) {
            saved_resolution.first = data["saved_resolution"]["width"].get<int>();
            saved_resolution.second = data["saved_resolution"]["height"].get<int>();
            RES_ORIG = saved_resolution; // Update RES_ORIG with saved resolution
            resolution_loaded = true; // Mark that resolution was loaded
            show_progress("Saved resolution loaded: " +
                std::to_string(saved_resolution.first) + "x" +
                std::to_string(saved_resolution.second));
        }
        else {
            // If no saved resolution, use default (1920x1200)
            resolution_loaded = false; // No saved resolution found
            saved_resolution = { 0, 0 }; // Invalid saved resolution
            show_progress("No saved resolution found. Using default: " +
                std::to_string(RES_ORIG.first) + "x" +
                std::to_string(RES_ORIG.second));
        }

        // Load coordinates
        for (auto& item : data["coordinates"].items()) {
            std::string name = item.key();
            auto& info = item.value();

            coordinates[name] = {
                info["x"].get<int>(),
                info["y"].get<int>(),
                info["description"].get<std::string>()
            };
        }
        log_load("Coordinates loaded: " + std::to_string(coordinates.size()) + " coordinates");
    }
    catch (const std::exception& e) {
        log_error("Error loading coordinates: " + std::string(e.what()));
        resolution_loaded = false;
        saved_resolution = { 0, 0 };
    }
}

// Structure for schedules
struct Schedule {
    int hour;
    int minute;
    bool active;
    bool recurring;  // true = daily, false = single
    std::string description;
    std::time_t last_execution;  // For single execution control
};

// Global variables for schedules
std::vector<Schedule> schedules;
std::mutex schedules_mutex;
std::thread schedule_thread;
std::atomic<bool> schedule_running(false);

// Global flags
std::atomic<bool> stop_count(false);
std::atomic<bool> stop_global_flag(false);
std::atomic<bool> general_cycle_running(false);
std::map<std::string, std::atomic<bool>> stop_flags;
std::mutex stop_flags_mutex;

// Structure for accounts
struct Account {
    std::string name;
    std::string username;
    std::string password;
    bool heart_donor;
    int friend_count; // 0 if not donor, 1 to 10 if donor
};

const BYTE VK_F = 0x46;
const BYTE VK_R = 0x52;
const BYTE SCANCODE_F = 0x21;
const BYTE SCANCODE_R = 0x13;

std::vector<Account> accounts; // Empty vector - will be filled by JSON

const std::string STEAM_PATH = "C:\\Program Files (x86)\\Steam\\steam.exe";
const std::string STEAM_GAME_URL = "steam://rungameid/2325290";

// Logging
void log_message(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm bt = localtime_xp(now_time);

    std::stringstream ss;
    ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << " - " << message;

    std::cout << ss.str() << std::endl;

    // Modify this line to use get_log_path()
    std::ofstream logfile(get_log_path(), std::ios_base::app);
    if (logfile.is_open()) {
        logfile << ss.str() << std::endl;
    }
}

const std::string ACCOUNTS_FILE = "accounts.json";
const std::string SCHEDULES_FILE = "schedules.json";

void save_accounts_json() {
    json j;
    for (const auto& account : accounts) {
        j["accounts"].push_back({
            {"name", account.name},
            {"username", account.username},
            {"password", account.password},
            {"heart_donor", account.heart_donor},
            {"friend_count", account.friend_count}
            });
    }

    std::ofstream out(ACCOUNTS_FILE);
    if (out.is_open()) {
        out << j.dump(4);
        out.close();
        show_progress("Accounts saved to JSON file: " + std::to_string(accounts.size()) + " accounts");
    }
    else {
        show_progress("Error saving accounts to JSON file");
    }
}

void load_accounts_json() {
    std::string full_path = get_exe_path() + ACCOUNTS_FILE;
    std::ifstream in(full_path);
    accounts.clear(); // Clear vector before loading

    if (!in.is_open()) {
        log_load("File " + full_path + " not found. Creating new...");
        std::ofstream out(full_path);
        if (out.is_open()) {
            out << "{\"accounts\":[]}";
            out.close();
        }
        else {
            log_error("Error creating file " + full_path);
        }
        return;
    }

    try {
        json j;
        in >> j;
        in.close();

        if (!j.contains("accounts") || !j["accounts"].is_array()) {
            log_error("Invalid structure in file " + full_path + " - 'accounts' not found or not an array");
            return;
        }

        for (const auto& item : j["accounts"]) {
            if (item.contains("name") && item.contains("username") && item.contains("password") &&
                item.contains("heart_donor") && item.contains("friend_count")) {
                Account account;
                account.name = item["name"].get<std::string>();
                account.username = item["username"].get<std::string>();
                account.password = item["password"].get<std::string>();
                account.heart_donor = item["heart_donor"].get<bool>();
                account.friend_count = item["friend_count"].get<int>();
                accounts.push_back(account);
            }
            else {
                log_error("Invalid account item in file " + full_path);
            }
        }
        log_load("Accounts loaded: " + std::to_string(accounts.size()) + " accounts found");
    }
    catch (const std::exception& e) {
        log_error("ERROR loading accounts: " + std::string(e.what()));
        accounts.clear();
    }
}

void save_schedules_json() {
    json j;
    for (const auto& sched : schedules) {
        j["schedules"].push_back({
            {"hour", sched.hour},
            {"minute", sched.minute},
            {"active", sched.active},
            {"recurring", sched.recurring},
            {"description", sched.description},
            {"last_execution", sched.last_execution}
            });
    }

    std::ofstream out(SCHEDULES_FILE);
    out << j.dump(4);
    out.close();
}

void load_schedules_json() {
    std::string full_path = get_exe_path() + SCHEDULES_FILE;
    std::ifstream in(full_path);

    if (!in.good()) return;

    json j;
    in >> j;
    in.close();

    schedules.clear();
    for (const auto& item : j["schedules"]) {
        schedules.push_back({
            item["hour"].get<int>(),
            item["minute"].get<int>(),
            item["active"].get<bool>(),
            item["recurring"].get<bool>(),
            item["description"].get<std::string>(),
            item["last_execution"].get<std::time_t>()
            });
    }
}

// Process management
bool is_process_running(const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }

    do {
        if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0) {
            CloseHandle(hSnapshot);
            return true;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return false;
}

// 1. First function prototypes (declarations)
void kill_process(const std::wstring& processName);
void close_game();
void close_steam();
void close_steam_completely();

// 2. Then function implementations
void kill_process(const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return;
    }

    do {
        if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

void close_game() {
    kill_process(L"Sky.exe");
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void close_steam() {
    system("taskkill /IM steam.exe /F > nul 2>&1");
    system("taskkill /IM steamwebhelper.exe /F > nul 2>&1");
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void close_steam_completely() {
    const std::vector<std::wstring> processes = {
        L"steam.exe",
        L"steamwebhelper.exe",
        L"steamservice.exe",
        L"Sky.exe"
    };

    for (const auto& process : processes) {
        for (int i = 0; i < 3; i++) {  // 3 attempts
            kill_process(process);
            if (!is_process_running(process)) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
}

// Window management
HWND find_window_by_title(const std::wstring& title) {
    return FindWindowW(NULL, title.c_str());
}

bool is_window_active(HWND hwnd) {
    return GetForegroundWindow() == hwnd;
}

void maximize_window(HWND hwnd) {
    if (hwnd == nullptr) {
        log_error("Invalid window handle in maximize_window");
        return;
    }
    ShowWindow(hwnd, SW_MAXIMIZE);
    SetForegroundWindow(hwnd); // Ensure window is brought to front
    log_load("Window maximized successfully");    
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

bool sky_window_active() {
    HWND hwnd = find_window_by_title(L"Sky");
    if (!hwnd) {
        log_error("Sky window not identified");
        return false;
    }
    return is_window_active(hwnd);
}

bool ensure_window_active(HWND hwnd) {
    if (!hwnd) {
        hwnd = find_window_by_title(L"Sky");
        if (!hwnd) {
            log_error("Sky window not found to activate");
            return false;
        }
    }

    if (!is_window_active(hwnd)) {
        show_progress("Activating game window...");
        SetForegroundWindow(hwnd);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Check if window was successfully activated
        if (!is_window_active(hwnd)) {
            log_error("Failed to activate game window");
            return false;
        }
    }
    return true;
}

// Internet check
bool check_connection() {
    clear_screen();
    show_progress("Checking internet connection");
    bool connected = InternetCheckConnection(L"https://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0);
    if (!connected) {
        log_error("No internet connection");
        return false;
    }
    show_progress("Internet connection verified");
    return true;
}

// Mouse simulation
void mouse_move(int x, int y) {
    auto coords = adjust_coordinates(x, y);
    SetCursorPos(coords.first, coords.second);
}

void mouse_click() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Function to generate random numbers in a range
int random_range(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

// Human mouse movement between two points
void precise_click(int x, int y, int steps = 50, int duration_ms = 500) {
    POINT current_pos;
    GetCursorPos(&current_pos); // Corrigido: removido caractere inválido ¤t_pos
    int start_x = current_pos.x;
    int start_y = current_pos.y;

    // Add some randomness to parameters
    steps += random_range(-10, 10);
    duration_ms += random_range(-100, 100);

    // Bezier curve for more natural movement
    for (int i = 0; i <= steps; i++) {
        float t = static_cast<float>(i) / static_cast<float>(steps);

        // Smooth with Bezier curve (ease-in-out)
        t = t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;

        // Add small random variations to path
        int rand_offset_x = (i % 5 == 0) ? random_range(-3, 3) : 0;
        int rand_offset_y = (i % 7 == 0) ? random_range(-3, 3) : 0;

        int x_pos = start_x + static_cast<int>((x - start_x) * t) + rand_offset_x;
        int y_pos = start_y + static_cast<int>((y - start_y) * t) + rand_offset_y;

        SetCursorPos(x_pos, y_pos);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms / steps));
    }

    // Ensure we reach final destination
    SetCursorPos(x, y);

    // Small pause after movement
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(50, 200)));
}

void human_mouse_move(int x, int y) {
    POINT start_pos;
    GetCursorPos(&start_pos);

    // Bezier curve for natural movement
    for (float t = 0; t <= 1; t += 0.02f) {
        int rand_offset = (rand() % 3) - 1; // Variation from -1 to +1 pixel
        int x_pos = start_pos.x + static_cast<int>((x - start_pos.x) * t) + rand_offset;
        int y_pos = start_pos.y + static_cast<int>((y - start_pos.y) * (t * t * (3 - 2 * t))) + rand_offset;
        SetCursorPos(x_pos, y_pos);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

// Human click with small variations
void human_click() {
    // Small movement before click (like a human adjusting)
    POINT pos;
    GetCursorPos(&pos);
    SetCursorPos(pos.x + random_range(-1, 1), pos.y + random_range(-1, 1));

    // Variable press time
    int press_time = random_range(20, 100);

    // Press button
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(press_time));

    // Release button
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    // Small pause after click
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(50, 200)));
}

void receive_hearts() {
    precise_click(coordinates["receive_hearts"].x, coordinates["receive_hearts"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
}

void prohibit_teleport() {
    precise_click(coordinates["social"].x, coordinates["social"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    precise_click(coordinates["prohibit_teleport"].x, coordinates["prohibit_teleport"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    
}

void donate_hearts() {
    precise_click(coordinates["social"].x, coordinates["social"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    precise_click(coordinates["friend_1"].x, coordinates["friend_1"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["send_light_friend_1"].x, coordinates["send_light_friend_1"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["donate_heart_friend_1"].x, coordinates["donate_heart_friend_1"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["random_social_click"].x, coordinates["random_social_click"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void donate_hearts_2() {
    precise_click(coordinates["social"].x, coordinates["social"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    precise_click(coordinates["friend_1"].x, coordinates["friend_1"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["send_light_friend_1"].x, coordinates["send_light_friend_1"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["donate_heart_friend_1"].x, coordinates["donate_heart_friend_1"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["random_social_click"].x, coordinates["random_social_click"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["friend_2"].x, coordinates["friend_2"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["send_light_friend_2"].x, coordinates["send_light_friend_2"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["donate_heart_friend_2"].x, coordinates["donate_heart_friend_2"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    precise_click(coordinates["random_social_click"].x, coordinates["random_social_click"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void donate_hearts_3() {
    //...implement
}

void donate_hearts_4() {
    //...implement
}

void donate_hearts_5() {
    //...implement
}

void donate_hearts_6() {
    //...implement
}

void donate_hearts_7() {
    //...implement
}

void donate_hearts_8() {
    //...implement
}

void donate_hearts_9() {
    //...implement
}

void donate_hearts_10() {
    //...implement
}

void press_key(int key, int duration_ms = 50) {  // Now accepts int (char or VK_*)
    HWND hwnd = find_window_by_title(L"Sky");
    if (!ensure_window_active(hwnd)) {
        log_error("Could not activate window to press key");
        return;
    }

    // Key to scan code mapping
    static const std::map<char, std::pair<BYTE, BYTE>> key_mapping = {
    {'F', {VK_F, SCANCODE_F}},
    {'f', {VK_F, SCANCODE_F}},
    {'R', {VK_R, SCANCODE_R}},
    {'r', {VK_R, SCANCODE_R}},
    {' ', {VK_SPACE, 0x39}},  // Space (scancode 0x39)
    {VK_HOME, {VK_HOME, 0x47}}   // Home (scancode 0x47, using fictional char 0x01)
    };

    auto it = key_mapping.find(key);
    if (it == key_mapping.end()) {
        log_error("Key '" + std::string(1, key) + "' not mapped");
        return;
    }

    BYTE vk = it->second.first;
    BYTE scan = it->second.second;

    log_load("Pressing key '" + std::string(1, key) + "'");

    // Press key using scan code
    INPUT input_down = { 0 };
    input_down.type = INPUT_KEYBOARD;
    input_down.ki.wScan = scan;
    input_down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT input_up = input_down;
    input_up.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(1, &input_down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    SendInput(1, &input_up, sizeof(INPUT));

    // Small pause after pressing key
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(50, 200)));
}

void collect_daily_quests() {
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    precise_click(coordinates["teleport"].x, coordinates["teleport"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    precise_click(coordinates["custom"].x, coordinates["custom"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    precise_click(coordinates["custom_coordinate"].x, coordinates["custom_coordinate"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    std::this_thread::sleep_for(std::chrono::seconds(5));

    precise_click(coordinates["random_game_click"].x, coordinates["random_game_click"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    // Ensure window is active before pressing F
    HWND hwnd = find_window_by_title(L"Sky");
    if (ensure_window_active(hwnd)) {
        press_key('F', random_range(30, 80));
        std::this_thread::sleep_for(std::chrono::milliseconds(random_range(100, 300)));
        press_key('F', random_range(30, 80));
    }
    else {
        log_error("Could not press F - window not active");
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void complete_daily_quests() {
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    precise_click(coordinates["unlocks"].x, coordinates["unlocks"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    precise_click(coordinates["complete_daily_missions"].x, coordinates["complete_daily_missions"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

void access_progression_menu() {
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    precise_click(coordinates["progression"].x, coordinates["progression"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
}

void receive_candles_quests() {
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    // Ensure window is active before pressing F
    HWND hwnd = find_window_by_title(L"Sky");
    if (ensure_window_active(hwnd)) {
        press_key('F', random_range(30, 80));
        std::this_thread::sleep_for(std::chrono::milliseconds(random_range(100, 300)));
        press_key('F', random_range(30, 80));
    }
    else {
        log_error("Could not press F - window not active");
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void receive_candles() {
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));
    precise_click(coordinates["random_game_click"].x, coordinates["random_game_click"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    // Ensure window is active before pressing R
    HWND hwnd = find_window_by_title(L"Sky");
    if (ensure_window_active(hwnd)) {
        press_key('R', random_range(30, 80));
        std::this_thread::sleep_for(std::chrono::milliseconds(random_range(100, 300)));
        press_key('R', random_range(30, 80));
    }
    else {
        log_error("Could not press R - window not active");
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
}

void hide_mod() {
    HWND hwnd = find_window_by_title(L"Sky");
    if (ensure_window_active(hwnd)) {
        // Press Home key
        press_key(VK_HOME, 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(random_range(100, 300)));
    }
    else {
        log_error("Could not hide mod - window not active");
    }
}

bool restart_game_in_progress = false;

void start_candle_run(const std::string& account_name) {
    precise_click(coordinates["start_automatic_wax_collection"].x, coordinates["start_automatic_wax_collection"].y);
    human_click();
    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(400, 600)));

    precise_click(coordinates["start_candle_run_from_beginning"].x, coordinates["start_candle_run_from_beginning"].y);
    human_click();
    show_progress("Candle Run - In progress");
    hide_mod();

    const int total_time = 15 * 60;
    const int interval = 5;
    const int max_attempts = 2;
    int attempts = 0;

    for (int i = 0; i < total_time; i += interval) {
        {
            std::lock_guard<std::mutex> lock(stop_flags_mutex);
            if (stop_global_flag) {
                log_error("Candle Run - Stopped by user");
                return;
            }
        }

        // Check if Sky process is still running
        if (!is_process_running(L"Sky.exe")) {
            log_error("Sky.exe process was closed - Marking to restart");
            restart_game_in_progress = true;
            return;  // Return immediately to restart flow
        }

        try {
            if (!sky_window_active()) {
                attempts++;
                log_error("Inactive window - Attempt " + std::to_string(attempts) + "/" + std::to_string(max_attempts));

                HWND hwnd = find_window_by_title(L"Sky");
                if (hwnd) {
                    precise_click(100, 100);
                    human_click();

                    SetForegroundWindow(hwnd);
                    std::this_thread::sleep_for(std::chrono::milliseconds(random_range(2500, 3500)));

                    if (sky_window_active()) {
                        log_load("Window reactivated successfully");
                        attempts = 0;
                        continue;
                    }
                }

                if (attempts >= max_attempts) {
                    log_error("Max attempts reached - Checking if game was closed");
                    if (!is_process_running(L"Sky.exe")) {
                        log_error("Game was closed - Marking to restart");
                        restart_game_in_progress = true;
                    }
                    else {
                        // If window is not active but process is still running
                        log_error("Inactive window but process still exists - Forcing restart");
                        close_game();  // Force game close
                        restart_game_in_progress = true;
                    }
                    return;  // Return immediately to restart flow
                }
            }
            else {
                attempts = 0;
            }

            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
        catch (const std::exception& e) {
            log_error("CRITICAL ERROR: " + std::string(e.what()));
            throw;
        }
    }
}


// Função para verificar pixel preto
bool is_pixel_black(int x, int y, int tolerance = 10) {
    auto adjusted = adjust_coordinates(x, y);
    HDC hdc = GetDC(NULL);
    COLORREF color = GetPixel(hdc, adjusted.first, adjusted.second);
    ReleaseDC(NULL, hdc);

    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);

    // Check if color is within tolerance of black (RGB: 0, 0, 0)
    return (r >= 0 && r <= tolerance) &&
        (g >= 0 && g <= tolerance) &&
        (b >= 0 && b <= tolerance);
}


// Função start_game corrigida
void start_game(const std::string& name, const std::string& user, const std::string& password) {
    show_progress("Starting game for " + name);
    close_game();
    close_steam();

    try {
        show_progress("Logging into Steam");
        std::string login_command = "start \"\" \"" + STEAM_PATH + "\" -login " + user + " " + password;
        system(login_command.c_str());

        std::this_thread::sleep_for(std::chrono::seconds(15));

        show_progress("Starting Sky game");
        std::string command = "start steam://rungameid/2325290";
        system(command.c_str());
    }
    catch (...) {
        log_error("Failed to start Steam");
        throw;
    }

    // Sky window verification
    log_load("Waiting for Sky window to be ready");
    HWND hwnd = nullptr;
    const int max_wait_seconds = 120;
    const int check_interval_ms = 1000;
    int elapsed_time = 0;

    while (elapsed_time < max_wait_seconds * 1000) {
        hwnd = find_window_by_title(L"Sky");
        if (hwnd) {
            wchar_t window_title[256];
            GetWindowTextW(hwnd, window_title, sizeof(window_title) / sizeof(wchar_t));
            std::wstring title(window_title);
            if (title == L"Sky") {
                log_load("Sky window ready");
                break;
            }
            else {
                log_load("Window not ready yet: " + wstring_to_string(title));
            }
        }
        else {
            log_load("Sky window not found");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms));
        elapsed_time += check_interval_ms;
    }

    if (!hwnd || elapsed_time >= max_wait_seconds * 1000) {
        log_error("Sky window not found or didn't become ready in time");
        throw std::runtime_error("Failed to wait for Sky window");
    }

    log_load("Maximizing game window");
    maximize_window(hwnd);
    

    show_progress("Activating Mod");
    precise_click(coordinates["activating_mod"].x, coordinates["activating_mod"].y);
    mouse_click();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    try {
        show_progress("Starting Game (Play)");
        if (ensure_window_active(hwnd)) {
            for (int i = 0; i < 3; i++) {
                press_key(' ', 30);
                Sleep(random_range(100, 300));
            }
        }
        else {
            log_error("Could not press space - window not active");
        }

        // Wait for loading screen (black pixel) to disappear
        show_progress("Waiting for game to load ...");
        const int max_wait_loading = 300; // 5 minutes max wait
        const int check_interval = 1000; // Check every second
        int wait_time = 0;
        bool is_loading = true;

        while (is_loading && wait_time < max_wait_loading * 1000) {
            if (is_pixel_black(400, 1120)) {
                log_load("Loading screen still present");
            }
            else {
                log_load("Loading screen gone");
                is_loading = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(check_interval));
            wait_time += check_interval;

            // Check if stopped
            if (stop_global_flag) {
                log_error("Game loading interrupted by stop flag");
                return;
            }
        }

        if (is_loading) {
            log_error("Timeout waiting for black pixel to disappear");
            throw std::runtime_error("Game loading timeout");
        }

        // Additional small pause after loading
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Find account to check heart_donor and friend_count
        auto it = std::find_if(accounts.begin(), accounts.end(),
            [&name](const Account& c) { return c.name == name; });
        if (it == accounts.end()) {
            log_error("Account " + name + " not found in accounts list");
            throw std::runtime_error("Account not found");
        }

        // Execute heart donation or prohibit teleport based on account type
        if (it->heart_donor && it->friend_count >= 1 && it->friend_count <= 2) {
            show_progress("Donating Hearts");
            switch (it->friend_count) {
            case 1: donate_hearts(); break;
            case 2: donate_hearts_2(); break;
            case 3: donate_hearts_3(); break;
            case 4: donate_hearts_4(); break;
            case 5: donate_hearts_5(); break;
            case 6: donate_hearts_6(); break;
            case 7: donate_hearts_7(); break;
            case 8: donate_hearts_8(); break;
            case 9: donate_hearts_9(); break;
            case 10: donate_hearts_10(); break;
            }
        }
        else {
            show_progress("Prohibiting Teleport");
            prohibit_teleport();
        }

        if (name == "Main") {
            
            receive_hearts();
            show_progress("Completing Daily Quests");
            collect_daily_quests();
            press_key('F', random_range(30, 80));
            std::this_thread::sleep_for(std::chrono::milliseconds(random_range(100, 300)));
            press_key('F', random_range(30, 80));
            std::this_thread::sleep_for(std::chrono::milliseconds(random_range(100, 300)));
            complete_daily_quests();
            show_progress("Collecting Quest Candles");
            receive_candles_quests();
            access_progression_menu();
            start_candle_run(name);
            show_progress("Collecting Candles");
            receive_candles();
        }
        else {
            show_progress("Prohibiting Teleport");
            prohibit_teleport();
            show_progress("Completing Daily Quests");
            collect_daily_quests();
            complete_daily_quests();
            show_progress("Collecting Quest Candles");
            receive_candles_quests();
            access_progression_menu();
            start_candle_run(name);
            show_progress("Collecting Candles");
            receive_candles();
        }
    }
    catch (...) {
        throw;
    }
}

void steam_game_workflow(const std::string& name, const std::string& user, const std::string& password, bool individual) {
    show_progress("Starting automation for " + name + " (" + user + ")");

    int attempts = 0;
    const int max_attempts = 3;

    while (attempts < max_attempts) {
        restart_game_in_progress = false;

        try {
            {
                std::lock_guard<std::mutex> lock(stop_flags_mutex);
                if (stop_global_flag || (individual && stop_flags[name])) {
                    log_error("Automation stopped for " + name);
                    return;
                }
            }

            if (!check_connection()) {
                log_error("No connection - Aborting");
                return;
            }

            close_steam();
            close_game();

            start_game(name, user, password);

            // If marked to restart, go back to start of flow
            if (restart_game_in_progress) {
                show_progress("Restarting flow for " + name + " after game closed");
                continue;  // Restart while loop without incrementing attempts
            }

            // If we got here, flow completed successfully
            break;
        }
        catch (const std::exception& e) {
            attempts++;
            log_error("Attempt " + std::to_string(attempts) + "/" + std::to_string(max_attempts) + " for " + name + ": " + e.what());

            if (attempts >= max_attempts) {
                log_error("Failed after " + std::to_string(max_attempts) + " attempts for " + name);
                break;
            }

            close_game();
            close_steam();
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
        catch (...) {
            log_error("Unexpected error in " + name);
            break;
        }
    }

    show_progress("Automation finished for " + user);
    close_game();
    close_steam();

    if (individual) {
        std::lock_guard<std::mutex> lock(stop_flags_mutex);
        stop_flags.erase(name);
    }
}

void start_automation(const std::string& name, const std::string& user, const std::string& password, bool individual = true) {
    std::lock_guard<std::mutex> lock(stop_flags_mutex);

    if (individual) {
        stop_flags[name] = false;
    }
    else {
        stop_global_flag = false;
    }

    std::thread([name, user, password, individual]() {
        steam_game_workflow(name, user, password, individual);
        }).detach();
}

void general_cycle() {
    if (general_cycle_running) {
        log_error("Cycle already running");
        return;
    }

    general_cycle_running = true;
    stop_global_flag = false;
    show_progress("Starting general cycle...");

    std::thread([]() {
        try {
            for (const auto& account : accounts) {
                if (stop_global_flag) {
                    break;
                }

                // Create a thread for current account
                std::atomic<bool> account_finished{ false };
                std::thread([&account, &account_finished]() {
                    steam_game_workflow(account.name, account.username, account.password, false);
                    account_finished = true;
                    }).detach();

                // Wait until current account finishes or is stopped
                while (!account_finished && !stop_global_flag) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));

                    // Check if game was unexpectedly closed
                    if (!is_process_running(L"Sky.exe") && !account_finished) {
                        log_error("Game unexpectedly closed - Restarting flow for " + account.name);
                        break;
                    }
                }

                // Small pause between accounts
                if (!stop_global_flag) {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
        }
        catch (...) {
            log_error("Error in general cycle");
        }

        general_cycle_running = false;
        show_progress("Cycle finished");
        }).detach();
}

void stop_general_cycle() {
    stop_global_flag = true;
    {
        std::lock_guard<std::mutex> lock(stop_flags_mutex);
        for (auto& flag : stop_flags) {
            flag.second = true;
        }
    }
    show_progress("Stopping all processes...");
}

void stop_account(const std::string& name) {
    std::lock_guard<std::mutex> lock(stop_flags_mutex);
    stop_flags[name] = true;
    show_progress("Stopping account: " + name);
}

// Function to display styled header
void header() {
    system("cls");
    std::cout << "=============================================\n";
    std::cout << "      AUTO TSM - AUTOMATION FOR TSM (SKY)\n";
    std::cout << "=============================================\n";
    std::cout << " Version: 1.0       Developer: Scr1p7\n";
    std::cout << "=============================================\n\n";
}

void list_accounts() {
    header();
    std::cout << "---------------- ACCOUNT LIST -----------------\n\n";


    for (size_t i = 0; i < accounts.size(); i++) {
        // Account number (2 digits, right aligned)
        std::cout << " " << std::setw(2) << std::right << (i + 1) << ". ";

        // Account name (max 15 chars, left aligned)
        std::cout << std::left << std::setw(15) << accounts[i].name;

        // User (max 15 chars, left aligned)
        std::cout << " | User: " << std::setw(15) << accounts[i].username << " |\n";
    }

    std::cout << "--------------------------------------------------\n";
}

// Functions to manage accounts
void add_account() {
    Account new_account;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Account name: ";
    std::getline(std::cin, new_account.name);
    if (new_account.name.empty()) {
        std::cout << "Name cannot be empty!\n";
        pause();
        return;
    }

    std::cout << "Steam username: ";
    std::getline(std::cin, new_account.username);
    if (new_account.username.empty()) {
        std::cout << "Username cannot be empty!\n";
        pause();
        return;
    }

    std::cout << "Steam password: ";
    std::getline(std::cin, new_account.password);
    if (new_account.password.empty()) {
        std::cout << "Password cannot be empty!\n";
        pause();
        return;
    }

    char option;
    std::cout << "Heart donor? (Y)es or (N)o: ";
    while (true) {
        std::cin >> option;
        option = toupper(option);
        if (option == 'Y' || option == 'N') break;
        std::cout << "Invalid option! Enter Y or N: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    new_account.heart_donor = (option == 'Y');
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (new_account.heart_donor) {
        int friends;
        std::cout << "Number of friends (1 to 2): ";
        while (!(std::cin >> friends) || friends < 1 || friends > 2) {
            std::cout << "Please enter a number between 1 and 2: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        new_account.friend_count = friends;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    else {
        new_account.friend_count = 0;
    }

    accounts.push_back(new_account);
    save_accounts_json();
    std::cout << "\nAccount added successfully!\n";
}

void remove_account() {
    if (accounts.empty()) {
        std::cout << "No accounts registered.\n";
        pause();
        return;
    }

    list_accounts();
    std::cout << "\n0. Back\n";
    std::cout << "Enter account number to remove: ";

    int choice;
    while (!(std::cin >> choice) || choice < 0 || choice > static_cast<int>(accounts.size())) {
        std::cout << "Invalid option! Enter a number between 0 and " << accounts.size() << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (choice != 0) {
        accounts.erase(accounts.begin() + (choice - 1));
        save_accounts_json();
        std::cout << "Account removed successfully!\n";
        pause();
    }
}

void save_resolution_json() {
    std::string full_path = get_exe_path() + "coordinates.json";
    std::ifstream file(full_path);
    json data;

    // Load existing file content
    if (file.is_open()) {
        try {
            file >> data;
        }
        catch (const std::exception& e) {
            log_error("Error reading coordinates.json to save resolution: " + std::string(e.what()));
        }
        file.close();
    }
    else {
        log_error("Error opening coordinates.json for reading. Creating new file.");
        data["coordinates"] = json::object(); // Initialize with empty object
    }

    // Update saved resolution
    data["saved_resolution"] = {
        {"width", RES_ORIG.first},
        {"height", RES_ORIG.second}
    };

    // Save file
    std::ofstream out(full_path);
    if (out.is_open()) {
        out << data.dump(4); // Format with indentation
        out.close();
        saved_resolution = RES_ORIG; // Update saved_resolution
        resolution_loaded = true; // Mark as loaded
        show_progress("Resolution saved in coordinates.json: " +
            std::to_string(RES_ORIG.first) + "x" +
            std::to_string(RES_ORIG.second));
    }
    else {
        log_error("Error saving resolution in coordinates.json");
    }
}

void add_schedule() {
    Schedule new_sched;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Description: ";
    std::getline(std::cin, new_sched.description);
    if (new_sched.description.empty()) {
        std::cout << "Description cannot be empty!\n";
        pause();
        return;
    }

    std::cout << "Hour (0-23): ";
    while (!(std::cin >> new_sched.hour) || new_sched.hour < 0 || new_sched.hour > 23) {
        std::cout << "Invalid hour! Enter a number between 0 and 23: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Minute (0-59): ";
    while (!(std::cin >> new_sched.minute) || new_sched.minute < 0 || new_sched.minute > 59) {
        std::cout << "Invalid minute! Enter a number between 0 and 59: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    char option;
    std::cout << "Recurring? (D)aily or (O)ne-time [D/O]: ";
    while (true) {
        std::cin >> option;
        option = toupper(option);
        if (option == 'D' || option == 'O') break;
        std::cout << "Invalid option! Enter D or O: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    new_sched.recurring = (option == 'D');

    new_sched.active = true;
    new_sched.last_execution = 0;

    {
        std::lock_guard<std::mutex> lock(schedules_mutex);
        schedules.push_back(new_sched);
        save_schedules_json();
    }

    std::cout << "\nSchedule added successfully!\n";
    pause();
}

void remove_schedule() {
    std::lock_guard<std::mutex> lock(schedules_mutex);

    if (schedules.empty()) {
        std::cout << "No schedules registered.\n";
        pause();
        return;
    }

    header();
    std::cout << ">> REGISTERED SCHEDULES <<\n\n";
    for (size_t i = 0; i < schedules.size(); i++) {
        const auto& sched = schedules[i];
        std::cout << " " << (i + 1) << ". " << std::setw(2) << std::setfill('0') << sched.hour << ":"
            << std::setw(2) << std::setfill('0') << sched.minute << " - " << sched.description
            << " [" << (sched.active ? "Active" : "Inactive") << "]\n";
    }

    std::cout << "\n0. Back\n";
    std::cout << "Enter schedule number to remove: ";

    int choice;
    if (!(std::cin >> choice) || choice < 0 || choice > static_cast<int>(schedules.size())) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Invalid option!\n";
        pause();
        return;
    }

    if (choice != 0) {
        schedules.erase(schedules.begin() + (choice - 1));
        save_schedules_json(); // <-- Save to JSON
        std::cout << "Schedule removed successfully!\n";
        pause();
    }
}

void list_schedules() {
    std::lock_guard<std::mutex> lock(schedules_mutex);

    header();
    std::cout << ">> REGISTERED SCHEDULES <<\n\n";

    if (schedules.empty()) {
        std::cout << "No schedules registered.\n";
    }
    else {
        for (size_t i = 0; i < schedules.size(); i++) {
            const auto& sched = schedules[i];
            std::cout << " " << (i + 1) << ". " << std::setw(2) << std::setfill('0') << sched.hour << ":"
                << std::setw(2) << std::setfill('0') << sched.minute << " - " << sched.description
                << " [" << (sched.recurring ? "Daily" : "One-time") << "]"
                << " [" << (sched.active ? "Active" : "Inactive") << "]\n";
        }
    }

    pause();
}

void edit_schedule() {
    std::lock_guard<std::mutex> lock(schedules_mutex);

    if (schedules.empty()) {
        std::cout << "No schedules registered.\n";
        pause();
        return;
    }

    header();
    std::cout << ">> EDIT SCHEDULE <<\n\n";
    for (size_t i = 0; i < schedules.size(); i++) {
        const auto& sched = schedules[i];
        std::cout << " " << (i + 1) << ". " << std::setw(2) << std::setfill('0') << sched.hour << ":"
            << std::setw(2) << std::setfill('0') << sched.minute << " - " << sched.description
            << " [" << (sched.active ? "Active" : "Inactive") << "]\n";
    }

    std::cout << "\n0. Back\n";
    std::cout << "Enter schedule number to edit: ";

    int choice;
    while (!(std::cin >> choice) || choice < 0 || choice > static_cast<int>(schedules.size())) {
        std::cout << "Invalid option! Enter a valid number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (choice != 0) {
        auto& sched = schedules[choice - 1];
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "New description [" << sched.description << "]: ";
        std::string new_desc;
        std::getline(std::cin, new_desc);
        if (!new_desc.empty()) sched.description = new_desc;

        std::cout << "New hour [" << sched.hour << "]: ";
        std::string hour_str;
        std::getline(std::cin, hour_str);
        if (!hour_str.empty()) {
            int new_hour;
            try {
                new_hour = std::stoi(hour_str);
                if (new_hour >= 0 && new_hour <= 23) sched.hour = new_hour;
                else std::cout << "Invalid hour! Keeping current value.\n";
            }
            catch (...) {
                std::cout << "Invalid input! Keeping current value.\n";
            }
        }

        std::cout << "New minute [" << sched.minute << "]: ";
        std::string minute_str;
        std::getline(std::cin, minute_str);
        if (!minute_str.empty()) {
            int new_minute;
            try {
                new_minute = std::stoi(minute_str);
                if (new_minute >= 0 && new_minute <= 59) sched.minute = new_minute;
                else std::cout << "Invalid minute! Keeping current value.\n";
            }
            catch (...) {
                std::cout << "Invalid input! Keeping current value.\n";
            }
        }

        std::cout << "Recurring? (D)aily or (O)ne-time [" << (sched.recurring ? "D" : "O") << "]: ";
        char option;
        while (true) {
            std::cin >> option;
            option = toupper(option);
            if (option == 'D' || option == 'O' || option == '\n') break;
            std::cout << "Invalid option! Enter D, O or Enter to keep: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        if (option == 'D' || option == 'O') sched.recurring = (option == 'D');
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "Active? (Y)es or (N)o [" << (sched.active ? "Y" : "N") << "]: ";
        while (true) {
            std::cin >> option;
            option = toupper(option);
            if (option == 'Y' || option == 'N' || option == '\n') break;
            std::cout << "Invalid option! Enter Y, N or Enter to keep: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        if (option == 'Y' || option == 'N') sched.active = (option == 'Y');
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        save_schedules_json();
        std::cout << "Schedule edited successfully!\n";
        pause();
    }
}

void sequential_general_cycle() {
    if (general_cycle_running) return;

    general_cycle_running = true;
    stop_global_flag = false;

    try {
        // Complete initial cleanup
        close_steam_completely();
        close_game();
        std::this_thread::sleep_for(std::chrono::seconds(10));

        for (const auto& account : accounts) {
            if (stop_global_flag) break;

            show_progress("STARTING ACCOUNT: " + account.name);

            int attempts = 0;
            bool success = false;

            while (attempts < 3 && !success && !stop_global_flag) {
                attempts++;
                restart_game_in_progress = false;

                // Direct execution without additional threads
                steam_game_workflow(account.name, account.username, account.password, false);

                if (!restart_game_in_progress) {
                    success = true;
                }
                else {
                    log_error("Attempt " + std::to_string(attempts) + " failed - Restarting");
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                }
            }

            // Pause between accounts
            if (!stop_global_flag && &account != &accounts.back()) {
                show_progress("Preparing for next account...");
                std::this_thread::sleep_for(std::chrono::seconds(15));

                // Cleanup between accounts
                close_steam_completely();
                close_game();
            }
        }
    }
    catch (...) {
        log_error("Error in sequential general cycle");
    }

    general_cycle_running = false;
    show_progress("SEQUENTIAL GENERAL CYCLE COMPLETED");
    std::exit(0);
}

// Thread to check schedules
void check_schedules() {
    schedule_running = true;

    while (schedule_running) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = localtime_xp(now_time);

        {
            std::lock_guard<std::mutex> lock(schedules_mutex);
            for (auto& sched : schedules) {
                if (sched.active && sched.hour == now_tm.tm_hour && sched.minute == now_tm.tm_min) {
                    // Check if already executed today (to avoid multiple executions)
                    std::tm last_tm = localtime_xp(sched.last_execution);
                    bool same_day = (last_tm.tm_year == now_tm.tm_year &&
                        last_tm.tm_mon == now_tm.tm_mon &&
                        last_tm.tm_mday == now_tm.tm_mday);

                    if (!sched.recurring && sched.last_execution != 0) {
                        // One-time schedule already executed
                        sched.active = false;
                        continue;
                    }

                    if (sched.recurring || !same_day) {
                        show_progress("Executing schedule: " + sched.description);
                        sched.last_execution = now_time;

                        // Complete closure before starting
                        close_steam_completely();
                        close_game();
                        std::this_thread::sleep_for(std::chrono::seconds(10));

                        // Safe execution
                        if (!general_cycle_running) {
                            std::thread([]() {
                                sequential_general_cycle();
                                }).detach();
                        }

                        // If one-time, deactivate after executing
                        if (!sched.recurring) {
                            sched.active = false;
                        }

                        // Wait to avoid multiple executions
                        std::this_thread::sleep_for(std::chrono::minutes(1));
                        break;
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}

void accounts_menu() {
    int option;
    do {
        header();
        std::cout << ">> MANAGE ACCOUNTS <<\n\n";
        std::cout << ">> Name the account as Main if you want to collect received hearts. <<\n\n";
        std::cout << "1. Add account\n";
        std::cout << "2. Remove account\n";
        std::cout << "3. List accounts\n";
        std::cout << "0. Back\n\n";
        std::cout << "Enter your option: ";

        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            continue;
        }

        switch (option) {
        case 1: add_account(); pause(); break;
        case 2: remove_account(); break;
        case 3: list_accounts(); pause();  break;
        case 0: return;
        default: break;
        }
    } while (option != 0);
}

void schedules_menu() {
    int option;
    do {
        header();
        std::cout << ">> MANAGE SCHEDULES <<\n\n";
        std::cout << "1. Add schedule\n";
        std::cout << "2. Remove schedule\n";
        std::cout << "3. Edit schedule\n";
        std::cout << "4. List schedules\n";
        std::cout << "0. Back\n\n";
        std::cout << "Enter your option: ";

        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            continue;
        }

        switch (option) {
        case 1: add_schedule(); break;
        case 2: remove_schedule(); break;
        case 3: edit_schedule(); break;
        case 4: list_schedules(); break;
        case 0: return;
        default: break;
        }
    } while (option != 0);
}

void main_menu() {
    if (!schedule_running) {
        schedule_thread = std::thread(check_schedules);
    }

    if (!resolution_loaded) {
        check_configure_resolution();
    }

    int option;
    do {
        header();
        std::cout << ">> MAIN MENU <<\n\n";
        std::cout << "1. Start general cycle (all accounts)\n";
        std::cout << "2. Run specific account\n";
        std::cout << "3. Manage accounts\n";
        std::cout << "4. Manage schedules\n";
        std::cout << "5. Configure resolution\n";
        std::cout << "6. Stop current execution\n";
        std::cout << "0. Exit\n\n";
        std::cout << "Enter your option: ";

        while (!(std::cin >> option) || option < 0 || option > 6) {
            std::cout << "Invalid option! Enter a number between 0 and 6: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        switch (option) {
        case 1:
            if (!general_cycle_running) {
                clear_screen();
                header();
                std::cout << "Preparing Cycle, please wait...\n\n";
                std::cout << "General cycle started.\n";
                std::cout << "Press Enter if you want to return to main menu...\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                std::thread(sequential_general_cycle).detach();
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
            }
            else {
                clear_screen();
                header();
                std::cout << "General cycle already running!\n";
                pause();
            }
            break;
        case 2: {
            int account_num;
            list_accounts();
            std::cout << "\n0. Back\n";
            std::cout << "Enter account number: ";

            while (!(std::cin >> account_num) || account_num < 0 || account_num > static_cast<int>(accounts.size())) {
                std::cout << "Invalid number! Enter a number between 0 and " << accounts.size() << ": ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            if (account_num != 0) {
                const auto& account = accounts[account_num - 1];
                start_automation(account.name, account.username, account.password);
                header();
                std::cout << "Account '" << account.name << "' started successfully!\n";
                std::cout << "Automation is ready to start.\n";
            }
            break;
        }
        case 3:
            accounts_menu();
            break;
        case 4:
            schedules_menu();
            break;
        case 5:
            check_configure_resolution();
            break;
        case 6:
            stop_general_cycle();
            pause();
            break;
        case 0:
            schedule_running = false;
            if (schedule_thread.joinable()) {
                schedule_thread.join();
            }
            return;
        }
    } while (true);
}

std::string get_log_path() {
    return get_exe_path() + "automation_log.txt";
}

void ensure_log_directory_exists() {
    std::string path = get_exe_path();
    if (!PathIsDirectoryA(path.c_str())) {
        CreateDirectoryA(path.c_str(), NULL);
    }
}

int main(int argc, char* argv[]) {
    std::string exe_path = get_exe_path();
    log_load("Executable directory: " + exe_path);

    // Check if files exist
    std::vector<std::string> required_files = {
        "coordinates.json",
        "accounts.json",
        "schedules.json"
    };

    for (const auto& file : required_files) {
        std::string full_path = exe_path + file;
        std::ifstream f(full_path);
        if (f.good()) {
            //log_message("File found: " + full_path);
        }
        else {
            //log_message("File NOT found: " + full_path);
        }
    }

    load_coordinates();
    load_accounts_json();
    load_schedules_json();

    // Check if there are accounts
    if (accounts.empty()) {
        log_error("No accounts registered. Add accounts through the menu.");
    }

    if (argc > 1 && std::string(argv[1]) == "/autostart") {
        sequential_general_cycle();

        while (general_cycle_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return 0;
    }

    main_menu();
    return 0;
}