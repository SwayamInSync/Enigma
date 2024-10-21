#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include "../include/Device.h"
#include "../include/DeviceType.h"
#include "../include/Storage.h"
#include "../include/COW.h"

using namespace enigma;

// Helper function to print test results
void print_test_result(const std::string& test_name, bool passed) {
    std::cout << test_name << ": " << (passed ? "PASSED" : "FAILED") << std::endl;
}

// Test basic Storage functionality
void test_storage_basics() {
    Device cpu_device(DeviceType::CPU);
    Storage storage(1000, cpu_device);
    
    bool passed = true;
    passed &= (storage.size_bytes() == 1000);
    passed &= (storage.device().type() == DeviceType::CPU);
    passed &= (storage.data() != nullptr);

    print_test_result("Storage Basics", passed);
}

// Test Storage resize
void test_storage_resize() {
    Device cpu_device(DeviceType::CPU);
    Storage storage(1000, cpu_device);
    void* original_data = storage.data();
    
    storage.resize(2000);
    
    bool passed = true;
    passed &= (storage.size_bytes() == 2000);
    passed &= (storage.data() != nullptr);
    passed &= (storage.data() != original_data);

    print_test_result("Storage Resize", passed);
}

// Test COW lazy_clone
void test_cow_lazy_clone() {
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);  // Fill with 1s
    
    auto clone = enigma::cow::lazy_clone_storage(*original);
    
    bool passed = true;
    passed &= (original->size_bytes() == clone->size_bytes());
    passed &= (original->data() == clone->data());  // Should point to the same data
    passed &= (std::memcmp(original->data(), clone->data(), 1000) == 0);  // Data should be identical
    passed &= enigma::cow::is_cow_data_ptr(original->data_ptr());
    passed &= enigma::cow::is_cow_data_ptr(clone->data_ptr());

    print_test_result("COW Lazy Clone", passed);
}

// Test COW materialization
void test_cow_materialization() {
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);  // Fill with 1s
    auto clone = enigma::cow::lazy_clone_storage(*original);
    enigma::cow::materialize_cow_storage(*clone);
    
    bool passed = true;
    passed &= (original->size_bytes() == clone->size_bytes());
    passed &= (original->data() != clone->data());  // Should point to different data now
    passed &= (std::memcmp(original->data(), clone->data(), 1000) == 0);  // Data should still be identical
    passed &= !enigma::cow::is_cow_data_ptr(clone->data_ptr());  // Clone should no longer be COW

    print_test_result("COW Materialization", passed);
}

// Test COW write behavior
void test_cow_write_behavior() {
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);  // Fill with 1s
    
    auto clone = enigma::cow::lazy_clone_storage(*original);
    
    // Modify the clone, which should trigger materialization
    enigma::cow::materialize_cow_storage(*clone);
    std::memset(clone->data(), 2, 1000);  // Fill clone with 2s
    
    bool passed = true;
    passed &= (std::memcmp(original->data(), clone->data(), 1000) != 0);  // Data should be different
    passed &= (*static_cast<char*>(original->data()) == 1);  // Original should still have 1s
    passed &= (*static_cast<char*>(clone->data()) == 2);  // Clone should have 2s

    print_test_result("COW Write Behavior", passed);
}

int main() {
    try {
        std::cout << "Starting Storage Basics test..." << std::endl;
        test_storage_basics();
        
        std::cout << "Starting Storage Resize test..." << std::endl;
        test_storage_resize();
        
        std::cout << "Starting COW Lazy Clone test..." << std::endl;
        test_cow_lazy_clone();
        
        std::cout << "Starting COW Materialization test..." << std::endl;
        test_cow_materialization();
        
        std::cout << "Starting COW Write Behavior test..." << std::endl;
        test_cow_write_behavior();
        
        std::cout << "All tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
        return 1;
    }
    
    return 0;
}