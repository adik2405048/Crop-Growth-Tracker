#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <numeric>

// --- Data Structures ---
// Using std::map as an equivalent to Python's dictionary.
// The key is the crop name (std::string) and the value is a std::vector,
// where each stage has a name (std::string) and duration (int).
std::map<std::string, std::vector<std::pair<std::string, int>>> CROP_DATA = {
    {"Paddy (Boro)", {
        {"Seedling Stage", 25}, {"Tillering Stage", 30}, {"Panicle Initiation", 30},
        {"Flowering Stage", 15}, {"Maturity & Ripening", 30}
    }},
    {"Wheat", {
        {"Germination & Seedling", 15}, {"Tillering Stage", 25}, {"Stem Extension (Jointing)", 25},
        {"Heading & Flowering", 20}, {"Grain Filling & Maturity", 30}
    }},
    {"Jute", {
        {"Seedling Establishment", 20}, {"Rapid Vegetative Growth", 60},
        {"Flowering & Pod Formation", 20}, {"Maturity", 20}
    }},
    {"Mustard", {
        {"Germination & Seedling", 15}, {"Vegetative Growth (Rosette)", 30},
        {"Flowering Stage", 25}, {"Pod Formation & Ripening", 25}
    }},
    {"Potato", {
        {"Sprouting", 15}, {"Vegetative Growth", 25},
        {"Tuber Initiation", 20}, {"Tuber Bulking & Maturity", 30}
    }}
};

// --- Helper Functions ---
// Function to convert a YYYY-MM-DD format string to a C++ time_point object.
std::chrono::system_clock::time_point parseDate(const std::string& date_str) {
    std::tm tm = {};
    std::stringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::runtime_error("Invalid date format. Please use YYYY-MM-DD.");
    }
    // Converting the tm struct to time_t and then to system_clock::time_point.
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

// Function to display a time_point object as a formatted string (e.g., "Sep 22").
std::string formatDate(const std::chrono::system_clock::time_point& time_point) {
    std::time_t tt = std::chrono::system_clock::to_time_t(time_point);
    std::tm ptm;
    // Use the appropriate function based on the operating system
    #if defined(_WIN32)
        localtime_s(&ptm, &tt); // Use localtime_s for Windows
    #else
        localtime_r(&tt, &ptm); // Use localtime_r for Linux/macOS
    #endif
    std::stringstream ss;
    ss << std::put_time(&ptm, "%b %d"); // Format: "Mon Day"
    return ss.str();
}

int main() {
    // --- Crop Selection ---
    std::cout << "--- Crop Growth Tracker (C++) ---\n";
    std::cout << "Select a crop:\n";
    std::vector<std::string> crop_names;
    int index = 1;
    for (const auto& pair : CROP_DATA) {
        std::cout << index << ". " << pair.first << std::endl;
        crop_names.push_back(pair.first);
        index++;
    }

    int choice;
    std::cout << "\nEnter your choice (1-" << crop_names.size() << "): ";
    std::cin >> choice;

    if (std::cin.fail() || choice < 1 || choice > crop_names.size()) {
        std::cerr << "Error: Invalid choice.\n";
        return 1;
    }
    std::string selected_crop = crop_names[choice - 1];

    // --- Sowing Date Input ---
    std::string sowing_date_str;
    std::cout << "Enter sowing date (YYYY-MM-DD): ";
    std::cin >> sowing_date_str;

    // --- Date Validation and Calculation ---
    std::chrono::system_clock::time_point sowing_date;
    try {
        sowing_date = parseDate(sowing_date_str);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    auto today = std::chrono::system_clock::now();
    
    // Calculating the difference in days.
    auto difference = today - sowing_date;
    long long days_since_sowing = std::chrono::duration_cast<std::chrono::hours>(difference).count() / 24;

    if (days_since_sowing < 0) {
        std::cout << "Information: The sowing date is in the future. Cannot track growth yet.\n";
        return 0;
    }

    const auto& crop_stages = CROP_DATA.at(selected_crop);
    int total_duration = 0;
    for(const auto& stage : crop_stages) {
        total_duration += stage.second;
    }

    std::string current_stage = "Harvest Ready / Cycle Complete";
    int cumulative_days = 0;
    int stage_start_day = 0;

    for (const auto& stage : crop_stages) {
        int duration = stage.second;
        if (days_since_sowing < cumulative_days + duration) {
            current_stage = stage.first;
            stage_start_day = cumulative_days;
            cumulative_days += duration;
            break;
        }
        cumulative_days += duration;
    }

    // --- Display Results ---
    std::cout << "\n--- Current Status ---\n";
    std::cout << "'" << selected_crop << "' was sown " << days_since_sowing << " days ago." << std::endl;
    std::cout << "Current Stage: " << current_stage << std::endl;

    if (current_stage != "Harvest Ready / Cycle Complete") {
        auto stage_start_date = sowing_date + std::chrono::hours(24 * stage_start_day);
        auto stage_end_date = sowing_date + std::chrono::hours(24 * (cumulative_days - 1));
        std::cout << "(From " << formatDate(stage_start_date) << " to " << formatDate(stage_end_date) << ")" << std::endl;
    }

    int progress_percent = (total_duration > 0) ? std::min(100, static_cast<int>((static_cast<double>(days_since_sowing) / total_duration) * 100)) : 100;
    
    std::cout << "\nOverall Progress: " << progress_percent << "%" << std::endl;
    std::cout << "[";
    for (int i = 0; i < 50; ++i) {
        if (i < progress_percent / 2) {
            std::cout << "#";
        } else {
            std::cout << "-";
        }
    }
    std::cout << "]\n";

    return 0;
}

