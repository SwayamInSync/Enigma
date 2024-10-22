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
void print_test_result(const std::string &test_name, bool passed)
{
    if (passed)
    {
        std::cout << test_name << " " << "\033[32mPassed\033[0m" << std::endl;
    }
    else
    {
        std::cout << test_name << " " << "\033[31mFailed\033[0m" << std::endl;
    }
}

// Test basic Storage functionality
void test_storage_basics()
{
    Device cpu_device(DeviceType::CPU);
    Storage storage(1000, cpu_device);

    bool passed = true;
    passed &= (storage.size_bytes() == 1000);
    passed &= (storage.device().type() == DeviceType::CPU);
    passed &= (storage.data() != nullptr);

    print_test_result("Storage Basics", passed);
}

// Test Storage resize
void test_storage_resize()
{
    Device cpu_device(DeviceType::CPU);
    Storage storage(1000, cpu_device);
    void *original_data = storage.data();

    storage.resize(2000);

    bool passed = true;
    passed &= (storage.size_bytes() == 2000);
    passed &= (storage.data() != nullptr);
    passed &= (storage.data() != original_data);

    print_test_result("Storage Resize", passed);
}

// Test COW lazy_clone and shared data
void test_cow_lazy_clone_and_shared_data()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000); // Fill with 1s

    auto clone1 = enigma::cow::lazy_clone_storage(*original);
    auto clone2 = enigma::cow::lazy_clone_storage(*original);
    std::cout << clone1->data() << " " << clone2->data() << " " << original->data() << std::endl;

    auto *ctx = static_cast<enigma::cow::COWDeleterContext *>(original->data_ptr().get_context());
    std::cout << ctx->reference_count() << std::endl;
    bool passed = true;
    passed &= (original->size_bytes() == clone1->size_bytes() && original->size_bytes() == clone2->size_bytes());
    passed &= (original->data() == clone1->data() && original->data() == clone2->data());                                             // Should point to the same data
    passed &= (std::memcmp(original->data(), clone1->data(), 1000) == 0 && std::memcmp(original->data(), clone2->data(), 1000) == 0); // Data should be identical
    passed &= enigma::cow::is_cow_data_ptr(original->data_ptr());
    passed &= enigma::cow::is_cow_data_ptr(clone1->data_ptr());
    passed &= enigma::cow::is_cow_data_ptr(clone2->data_ptr());

    print_test_result("COW Lazy Clone and Shared Data", passed);
}

// Test COW write behavior with multiple clones
void test_cow_write_behavior_multiple_clones()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000); // Fill with 1s

    auto clone1 = enigma::cow::lazy_clone_storage(*original);
    auto clone2 = enigma::cow::lazy_clone_storage(*original);

    // Modify clone1, which should trigger materialization
    std::memset(clone1->data(), 2, 1000); // Fill clone1 with 2s

    bool passed = true;
    passed &= (std::memcmp(original->data(), clone1->data(), 1000) != 0); // Data should be different
    passed &= (std::memcmp(original->data(), clone2->data(), 1000) == 0); // Original and clone2 should still be the same
    passed &= (*static_cast<char *>(original->data()) == 1);              // Original should still have 1s
    passed &= (*static_cast<char *>(clone1->data()) == 2);                // Clone1 should have 2s
    passed &= (*static_cast<char *>(clone2->data()) == 1);                // Clone2 should still have 1s
    passed &= (clone1->data() != clone2->data());                         // Clone1 and Clone2 should now point to different data

    print_test_result("COW Write Behavior with Multiple Clones", passed);
}

// Test that no copies are made until a write operation occurs
void test_cow_no_copy_until_write()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    void *original_data = original->data();

    auto clone1 = enigma::cow::lazy_clone_storage(*original);
    auto clone2 = enigma::cow::lazy_clone_storage(*original);

    bool passed = true;
    passed &= (original->data() == clone1->data() && original->data() == clone2->data()); // All should point to the same data

    // Perform a read operation
    volatile char dummy = *static_cast<char *>(clone1->data());
    (void)dummy; // Suppress unused variable warning

    passed &= (original->data() == clone1->data() && original->data() == clone2->data()); // Should still point to the same data after read

    // Perform a write operation
    *static_cast<char *>(clone1->data()) = 2;

    passed &= (original->data() == original_data); // Original should not have changed
    passed &= (clone1->data() != original_data);   // Clone1 should have new data
    passed &= (clone2->data() == original_data);   // Clone2 should still point to original data

    print_test_result("COW No Copy Until Write", passed);
}

// Test that when a Storage becomes unique, further modifications don't create new copies
void test_cow_unique_storage_modifications()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000); // Fill with 1s

    auto clone = enigma::cow::lazy_clone_storage(*original);

    // Modify clone, which should trigger materialization
    *static_cast<char *>(clone->data()) = 2;
    void *clone_data_after_first_write = clone->data();

    // Perform another modification on clone
    *static_cast<char *>(clone->data()) = 3;
    void *clone_data_after_second_write = clone->data();

    bool passed = true;
    passed &= (clone_data_after_first_write == clone_data_after_second_write); // Should not have created a new copy
    passed &= (*static_cast<char *>(original->data()) == 1);                   // Original should still have 1
    passed &= (*static_cast<char *>(clone->data()) == 3);                      // Clone should have 3

    print_test_result("COW Unique Storage Modifications", passed);
}

int main()
{
    try
    {
        std::cout << "Starting Storage Basics test..." << std::endl;
        test_storage_basics();

        std::cout << "Starting Storage Resize test..." << std::endl;
        test_storage_resize();

        std::cout << "Starting COW Lazy Clone and Shared Data test..." << std::endl;
        test_cow_lazy_clone_and_shared_data();

        std::cout << "Starting COW Write Behavior with Multiple Clones test..." << std::endl;
        test_cow_write_behavior_multiple_clones();

        std::cout << "Starting COW No Copy Until Write test..." << std::endl;
        test_cow_no_copy_until_write();

        std::cout << "Starting COW Unique Storage Modifications test..." << std::endl;
        test_cow_unique_storage_modifications();

        std::cout << "All tests completed successfully!" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
        return 1;
    }

    return 0;
}