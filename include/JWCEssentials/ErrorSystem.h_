// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
#ifndef ERRORSYSTEM_H
#define ERRORSYSTEM_H

namespace JWCEssentials {

    struct ErrorKeyValue {
        utf8_string_struct key;
        utf8_string_struct name;
        utf8_string_struct value;
    };

    struct ErrorInfo {
        P_ELEMENTS(ErrorKeyValue) details; // List of key-name-value triples
        size_t detail_count;

        utf8_string_struct message;           // Error message

        ErrorInfo(P_ELEMENTS(const ErrorKeyValue) details, size_t detail_count, utf8_string_struct msg);
        ~ErrorInfo();
    };

    extern _CLASSEXPORT_ TLS *JWCESSENTIALS_Errors;

    extern "C"
    {
        _EXPORT_ void LogError(P_ELEMENTS(const ErrorKeyValue)  details, size_t detail_count, utf8_string_struct message);
        _EXPORT_ void ClearErrors();
    }

    void _LogError(P_ELEMENTS(const ErrorKeyValue)  details, utf8_string_struct message);


    /* std::vector<KeyValue> errorDetails = {
    {"module", "CrystalCatalyst.DragDrop.X11", "void CrystalWindow_X11::RegisterDragTarget()"},
    {"facility", "Windowing", "0x00000000"},
    {"category", "unable to perform", "RegisterDragTarget"},
    {nullptr}
    };

    LogError(errorDetails, "Failed to register drag target.");



    Facilities
    const std::string Facility_Windowing = "Windowing";
    const std::string Facility_Input = "Input";
    const std::string Facility_Networking = "Networking";
    // Add more facilities as needed

    Categories
    const std::string Category_Performance = "Performance";
    const std::string Category_Initialization = "Initialization";
    const std::string Category_ResourceManagement = "ResourceManagement";
    // Add more categories as needed

    Facilities (to broaden scope across the system):
Facility_Rendering: For errors related to the rendering engine.
Facility_Audio: Handling audio-related issues.
Facility_Serialization: Issues when serializing/deserializing data.
Facility_IO: For file and stream input/output.
Facility_MemoryManagement: For errors related to memory allocation/deallocation.
Facility_Concurrency: Managing multi-threading or async operations.
Categories (capturing more specific error contexts):
Category_Timeout: Useful for signaling when an operation exceeded its allowed time.
Category_Validation: For input or state validation errors.
Category_Dependency: Signaling errors due to external dependencies or version mismatches.
Category_Protocol: For errors in communication protocols (e.g., network, IPC).
Category_Exception: Errors raised from uncaught exceptions in code.
*/
}
#endif //ERRORSYSTEM_H
