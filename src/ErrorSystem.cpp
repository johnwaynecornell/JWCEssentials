//
// Created by jwc on 10/13/24.
//

#include <list>
#include <map>

#include "JWCEssentials/JWCEssentials.h"

namespace JWCEssentials {
    struct ErrorLogEntry {
        utf8_string_struct errorMessage;
        utf8_string_struct errorCategory;
        utf8_string_struct moduleOfOrigin;     // Explicitly labeled as the module of origin
        utf8_string_struct timestamp;          // Can be more complex if desired

        ErrorLogEntry(const ErrorLogEntry &other) {
            errorMessage = other.errorMessage;
            errorCategory = other.errorCategory;
            timestamp = other.timestamp;
            moduleOfOrigin = other.moduleOfOrigin;
        }

        ErrorLogEntry(ErrorLogEntry &&other) noexcept {
            errorMessage = std::move(other.errorMessage);
            errorCategory = std::move(other.errorCategory);
            timestamp = std::move(other.timestamp);
            moduleOfOrigin = std::move(other.moduleOfOrigin);
        }

        ErrorLogEntry() {
            errorMessage = nullptr;
            errorCategory = nullptr;
            timestamp = nullptr;
            moduleOfOrigin = nullptr;
        }

        ErrorLogEntry &operator=(ErrorLogEntry &other) {
            errorMessage = other.errorMessage;
            errorCategory = other.errorCategory;
            timestamp = other.timestamp;
            moduleOfOrigin = other.moduleOfOrigin;

            return *this;

        }

        ErrorLogEntry &operator=(ErrorLogEntry &&other) noexcept{
            errorMessage = std::move(other.errorMessage);
            errorCategory = std::move(other.errorCategory);
            timestamp = std::move(other.timestamp);
            moduleOfOrigin = std::move(other.moduleOfOrigin);

            return *this;
        }
    };

    struct KeyValueEntry {
        utf8_string_struct Key;       // Category name (e.g., "IO Error")
        utf8_string_struct Value;     // Description of the category
        KeyValueEntry* next = nullptr; // Pointer to the next category
    };

    KeyValueEntry errorCategories;  // Dummy node, next points to the head of the list
    KeyValueEntry modules;  // Dummy node for module list

    KeyValueEntry * keyValueTail(KeyValueEntry *head) {
        while (head->next != nullptr) head = head->next;
        return head;
    }

    std::pmr::list<ErrorLogEntry> errorLog;

    void AddErrorCategory(const utf8_string_struct& key, const utf8_string_struct& description) {
        KeyValueEntry* newEntry = new KeyValueEntry();
        newEntry->Key = key;
        newEntry->Value = description;

        keyValueTail(&errorCategories)->next = newEntry;

    }

    void AddModule(const utf8_string_struct& moduleName, const utf8_string_struct& description) {
        KeyValueEntry* newModule = new KeyValueEntry();
        newModule->Key = moduleName;
        newModule->Value = description;

        keyValueTail(&modules)->next = newModule;
    }


    void InitErrorSystem() {

        AddErrorCategory("Resource Limit", "The system ran into resource allocation limits.");
        AddErrorCategory("Timeout Error", "An operation took longer than the allowed time.");
        AddErrorCategory("IO Error", "An input/output error occurred.");
        AddErrorCategory("Memory Overflow", "Memory allocation exceeded the available limit.");
        AddErrorCategory("Network Failure", "An error occurred while trying to communicate over the network.");
        AddErrorCategory("Unexpected Value", "An unexpected value was encountered during processing.");
    }

    void InitModuleSystem() {
        // Initialize with some default modules
        AddModule("Windowing", "Handles window creation and management.");
        AddModule("FileIO", "Handles file input/output operations.");
        AddModule("Networking", "Manages network connections and data transfers.");
    }

    utf8_string_struct GetModuleInfo(const utf8_string_struct& moduleName) {
        KeyValueEntry* current = modules.next;

        // Search for the module in the list
        while (current != nullptr) {
            if (current->Key == moduleName) {
                return current->Value;  // Return the description of the module
            }
            current = current->next;
        }

        // If module not found
        return "Unknown module";
    }


    std::map<std::string, int> errorCounts;

    void LogError(const utf8_string_struct& errorMessage, const utf8_string_struct& errorCategory, const utf8_string_struct& moduleOfOrigin) {
        // Create a new log entry
        ErrorLogEntry newEntry;
        newEntry.errorMessage = errorMessage;
        newEntry.errorCategory = errorCategory;     // Error category
        newEntry.moduleOfOrigin = moduleOfOrigin;   // Module of origin

        // Add a timestamp
        time_t now = time(0);
        newEntry.timestamp = ctime(&now);

        // Add to the error log
        int nextIndex = 0;  // Get the next available index
        errorLog.push_back(newEntry);

        // Optionally print the log for debugging
        std::cout << "Logged Error: " << errorMessage.c_str
                  << " | Category: " << errorCategory.c_str
                  << " | Module of Origin: " << moduleOfOrigin.c_str
                  << " | Time: " << newEntry.timestamp << std::endl;
    }


    void AggregateError(const utf8_string_struct& errorCategory) {
        errorCounts[errorCategory.c_str]++;
        std::cout << "Category: " << errorCategory.c_str << " has occurred " << errorCounts[errorCategory.c_str] << " times." << std::endl;
    }

    ErrorLogEntry GetErrorLogEntry(int index) {
        int e;

        for (std::list<ErrorLogEntry>::iterator i = errorLog.begin(); i != errorLog.end(); i++) {
            if (index-- == 0) return *i;
        }

        throw std::exception();
    }

    struct_array_struct<ErrorLogEntry> GetAllErrors() {
        int count = 0;
        for (std::list<ErrorLogEntry>::iterator i = errorLog.begin(); i != errorLog.end(); i++) {
            count++;
        }

        struct_array_struct<ErrorLogEntry> R;
        R.Alloc(count);

        int index;

        for (std::list<ErrorLogEntry>::iterator i = errorLog.begin(); i != errorLog.end(); i++) {
            R[index++] = *i;
        }

        return R;
    }

    int testErrorSystem() {
        InitErrorSystem();  // Initialize default error categories
        InitModuleSystem(); // Initialize default modules

        // Log errors with module of origin information
        LogError("File not found", "IO Error", "FileIO");
        LogError("Connection timeout", "Network Failure", "Networking");

        // Retrieve and display an error log entry
        ErrorLogEntry entry = GetErrorLogEntry(0);  // Get the first error
        std::cout << "Client Retrieved Error: " << entry.errorMessage.c_str << " | Category: " << entry.errorCategory.c_str << " | Time: " << entry.timestamp << std::endl;

        return 0;
    }

}