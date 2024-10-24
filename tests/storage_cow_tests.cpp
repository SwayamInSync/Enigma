#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include "../include/DataPtr.h"
#include "../include/Device.h"
#include "../include/DeviceType.h"
#include "../include/Storage.h"
#include "../include/COW.h"
#include "../include/DEBUG.h"

using namespace enigma;

struct TestOutput
{
    static void pass(const std::string &test_name)
    {
        std::cout << "[✓] " << test_name << " \033[32mPASSED\033[0m" << std::endl;
    }

    static void fail(const std::string &test_name, const std::string &reason)
    {
        std::cout << "[✗] " << test_name << " \033[31mFAILED\033[0m: " << reason << std::endl;
    }
};

// Test explicit materialization requirement
void test_cow_explicit_materialize()
{
    Device cpu_device(DeviceType::CPU);
    print("Making Original\n");
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);

    print("Making Clone\n");
    auto clone = cow::lazy_clone_storage(*original);

    assert(static_cast<unsigned char *>(clone->data())[0] == 1);
    bool passed = true;
    std::string fail_reason;

    // Modifying without materializing should affect all copies
    std::memset(clone->data(), 2, 1000);
    if (static_cast<unsigned char *>(original->data())[0] != 2)
    {
        passed = false;
        fail_reason = "Data not shared before materialization";
    }

    // Now explicitly materialize
    print("Materializing Clone\n");
    cow::materialize_cow_storage(*clone);
    std::memset(clone->data(), 3, 1000);

    // Original should still have 2s
    if (static_cast<unsigned char *>(original->data())[0] != 2)
    {
        passed = false;
        fail_reason = "Original changed after materialization";
    }

    // Clone should have 3s
    if (static_cast<unsigned char *>(clone->data())[0] != 3)
    {
        passed = false;
        fail_reason = "Clone not updated after materialization";
    }

    passed ? TestOutput::pass("COW Explicit Materialization")
           : TestOutput::fail("COW Explicit Materialization", fail_reason);
    print("Test Over, cleaning Up\n");
}

// Test sharing behavior without materialization
void test_cow_shared_modifications()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);

    auto clone1 = cow::lazy_clone_storage(*original);
    auto clone2 = cow::lazy_clone_storage(*original);

    bool passed = true;
    std::string fail_reason;

    // Modify clone1 without materializing
    std::memset(clone1->data(), 2, 1000);

    // All storages should see the change
    if (static_cast<unsigned char *>(original->data())[0] != 2 ||
        static_cast<unsigned char *>(clone2->data())[0] != 2)
    {
        passed = false;
        fail_reason = "Changes not visible to all copies before materialization";
    }
    // Materialize clone2 and modify
    cow::materialize_cow_storage(*clone2);
    std::memset(clone2->data(), 3, 1000);

    // clone2 should have different data
    if (static_cast<unsigned char *>(clone2->data())[0] != 3)
    {
        passed = false;
        fail_reason = "Clone2 not independent after materialization";
    }

    // original and clone1 should still share data
    if (static_cast<unsigned char *>(original->data())[0] != 2 ||
        static_cast<unsigned char *>(clone1->data())[0] != 2)
    {
        passed = false;
        fail_reason = "Original and clone1 not sharing data";
    }

    passed ? TestOutput::pass("COW Shared Modifications")
           : TestOutput::fail("COW Shared Modifications", fail_reason);
}

// Test reference counting with explicit materialization
void test_cow_refcount_with_materialization()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);

    bool passed = true;
    std::string fail_reason;

    auto clone1 = cow::lazy_clone_storage(*original);
    auto clone2 = cow::lazy_clone_storage(*original);

    auto *ctx = static_cast<cow::COWDeleterContext *>(original->data_ptr().get_context());
    int initial_refcount = ctx->reference_count();

    // Materialize clone1
    cow::materialize_cow_storage(*clone1);

    if (ctx->reference_count() != initial_refcount - 1)
    {
        passed = false;
        fail_reason = "Incorrect refcount after materialization";
    }

    passed ? TestOutput::pass("COW RefCount with Materialization")
           : TestOutput::fail("COW RefCount with Materialization", fail_reason);
}

// Test behavior of data access without materialization
void test_cow_data_access()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);

    // Fill with pattern
    for (size_t i = 0; i < 1000; i++)
    {
        static_cast<unsigned char *>(original->data())[i] = i % 256;
    }

    auto clone = cow::lazy_clone_storage(*original);
    bool passed = true;
    std::string fail_reason;

    // Reading without materialization
    if (std::memcmp(clone->data(), original->data(), 1000) != 0)
    {
        passed = false;
        fail_reason = "Data not identical in shared state";
    }

    // Modify through clone without materializing
    static_cast<unsigned char *>(clone->data())[0] = 0xFF;

    // Should affect original too
    if (static_cast<unsigned char *>(original->data())[0] != 0xFF)
    {
        passed = false;
        fail_reason = "Modifications not visible without materialization";
    }

    passed ? TestOutput::pass("COW Data Access")
           : TestOutput::fail("COW Data Access", fail_reason);
}

// Test cloning a materialized storage
void test_cow_clone_after_materialize()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone1 = cow::lazy_clone_storage(*original);

    // Materialize clone1
    cow::materialize_cow_storage(*clone1);

    // Now try to clone the materialized storage
    auto clone2 = cow::lazy_clone_storage(*clone1);
    bool passed = true;
    std::string fail_reason;

    // Modify clone2 without materializing
    std::memset(clone2->data(), 2, 1000);

    // clone1 should see changes (now they share COW)
    if (static_cast<unsigned char *>(clone1->data())[0] != 2)
    {
        passed = false;
        fail_reason = "Materialized storage not properly converted back to COW";
    }

    passed ? TestOutput::pass("COW Clone After Materialize")
           : TestOutput::fail("COW Clone After Materialize", fail_reason);
}

// Test zero-size storage
void test_cow_zero_size_storage()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(0, cpu_device);

    bool passed = true;
    std::string fail_reason;

    try
    {
        auto clone = cow::lazy_clone_storage(*original);
        cow::materialize_cow_storage(*clone);
        passed = true;
    }
    catch (const std::exception &e)
    {
        passed = false;
        fail_reason = "Failed to handle zero-size storage";
    }

    passed ? TestOutput::pass("COW Zero Size Storage")
           : TestOutput::fail("COW Zero Size Storage", fail_reason);
}

// Test multiple materializations
void test_cow_multiple_materialize()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone = cow::lazy_clone_storage(*original);

    bool passed = true;
    std::string fail_reason;

    // First materialization
    cow::materialize_cow_storage(*clone);
    void *first_data = clone->data();

    // Second materialization (should be no-op)
    cow::materialize_cow_storage(*clone);

    if (clone->data() != first_data)
    {
        passed = false;
        fail_reason = "Multiple materializations caused unnecessary data copy";
    }

    passed ? TestOutput::pass("COW Multiple Materialize")
           : TestOutput::fail("COW Multiple Materialize", fail_reason);
}

// Test chain of clones
void test_cow_clone_chain()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);

    bool passed = true;
    std::string fail_reason;

    auto clone1 = cow::lazy_clone_storage(*original);
    auto clone2 = cow::lazy_clone_storage(*clone1);
    auto clone3 = cow::lazy_clone_storage(*clone2);

    auto *ctx = static_cast<cow::COWDeleterContext *>(original->data_ptr().get_context());
    int ref_count = ctx->reference_count();

    if (ref_count != 4)
    { // original + 3 clones
        passed = false;
        fail_reason = "Incorrect reference count in clone chain: " + std::to_string(ref_count);
    }

    // Materialize middle of chain
    cow::materialize_cow_storage(*clone2);
    std::memset(clone2->data(), 2, 1000);

    // Original, clone1, and clone3 should still share data
    if (static_cast<unsigned char *>(original->data())[0] !=
            static_cast<unsigned char *>(clone1->data())[0] ||
        static_cast<unsigned char *>(original->data())[0] !=
            static_cast<unsigned char *>(clone3->data())[0])
    {
        passed = false;
        fail_reason = "Chain clone sharing broken after middle materialization";
    }

    passed ? TestOutput::pass("COW Clone Chain")
           : TestOutput::fail("COW Clone Chain", fail_reason);
}

// Test cleanup on destruction
void test_cow_cleanup()
{
    Device cpu_device(DeviceType::CPU);
    auto original = std::make_shared<Storage>(1000, cpu_device);

    bool passed = true;
    std::string fail_reason;

    {
        auto clone1 = cow::lazy_clone_storage(*original);
        auto *ctx = static_cast<cow::COWDeleterContext *>(original->data_ptr().get_context());
        int initial_count = ctx->reference_count();
        std::cout << "Initial count: " << initial_count << std::endl;

        {
            auto clone2 = cow::lazy_clone_storage(*original);
            std::cout << "After clone2: " << ctx->reference_count() << std::endl;
            if (ctx->reference_count() != initial_count + 1)
            {
                passed = false;
                fail_reason = "Reference count not increased with nested clone";
            }
        }
        std::cout << "After clone2 destroyed: " << ctx->reference_count() << std::endl;

        if (ctx->reference_count() != initial_count)
        {
            passed = false;
            fail_reason = "Reference count not decreased after clone destruction. Expected: " + std::to_string(initial_count) + " Got: " + std::to_string(ctx->reference_count());
        }
    }

    passed ? TestOutput::pass("COW Cleanup")
           : TestOutput::fail("COW Cleanup", fail_reason);
}

int main()
{
    try
    {
        std::cout << "\nRunning COW Tests...\n"
                  << std::endl;
        // Original tests
        std::cout << "*** Starting test: test_cow_explicit_materialize ***" << std::endl;
        test_cow_explicit_materialize();

        std::cout << "*** Starting test: test_cow_shared_modifications ***" << std::endl;
        test_cow_shared_modifications();

        std::cout << "*** Starting test: test_cow_refcount_with_materialization ***" << std::endl;
        test_cow_refcount_with_materialization();

        std::cout << "*** Starting test: test_cow_data_access ***" << std::endl;
        test_cow_data_access();

        // New tests
        std::cout << "*** Starting test: test_cow_clone_after_materialize ***" << std::endl;
        test_cow_clone_after_materialize();

        // std::cout << "*** Starting test: test_cow_zero_size_storage ***" << std::endl;
        // test_cow_zero_size_storage();

        std::cout << "*** Starting test: test_cow_multiple_materialize ***" << std::endl;
        test_cow_multiple_materialize();

        std::cout << "*** Starting test: test_cow_clone_chain ***" << std::endl;
        test_cow_clone_chain();

        std::cout << "*** Starting test: test_cow_cleanup ***" << std::endl;
        test_cow_cleanup();

        std::cout << "\nAll tests completed!\n"
                  << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n\033[31mException caught: " << e.what() << "\033[0m" << std::endl;
        return 1;
    }
    return 0;
}