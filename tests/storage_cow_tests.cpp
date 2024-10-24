#include <gtest/gtest.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include "DataPtr.h"
#include "Device.h"
#include "DeviceType.h"
#include "Storage.h"
#include "COW.h"
#include "DEBUG.h"

using namespace enigma;

// Test fixture for Storage/COW tests
class StorageTest : public ::testing::Test
{
protected:
    Device cpu_device;

    void SetUp() override
    {
        cpu_device = Device(DeviceType::CPU);
    }

    void TearDown() override {}
};

// Test explicit materialization requirement
TEST_F(StorageTest, ExplicitMaterialize)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);

    auto clone = cow::lazy_clone_storage(*original);
    EXPECT_EQ(static_cast<unsigned char *>(clone->data())[0], 1);

    // Modifying without materializing should affect all copies
    std::memset(clone->data(), 2, 1000);
    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0], 2)
        << "Data not shared before materialization";

    // Now explicitly materialize
    cow::materialize_cow_storage(*clone);
    std::memset(clone->data(), 3, 1000);

    // Original should still have 2s
    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0], 2)
        << "Original changed after materialization";

    // Clone should have 3s
    EXPECT_EQ(static_cast<unsigned char *>(clone->data())[0], 3)
        << "Clone not updated after materialization";
}

// Test sharing behavior without materialization
TEST_F(StorageTest, SharedModifications)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    std::memset(original->data(), 1, 1000);

    auto clone1 = cow::lazy_clone_storage(*original);
    auto clone2 = cow::lazy_clone_storage(*original);

    // Modify clone1 without materializing
    std::memset(clone1->data(), 2, 1000);

    // All storages should see the change
    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0], 2)
        << "Changes not visible to original before materialization";
    EXPECT_EQ(static_cast<unsigned char *>(clone2->data())[0], 2)
        << "Changes not visible to clone2 before materialization";

    // Materialize clone2 and modify
    cow::materialize_cow_storage(*clone2);
    std::memset(clone2->data(), 3, 1000);

    // clone2 should have different data
    EXPECT_EQ(static_cast<unsigned char *>(clone2->data())[0], 3)
        << "Clone2 not independent after materialization";

    // original and clone1 should still share data
    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0], 2)
        << "Original lost data sharing";
    EXPECT_EQ(static_cast<unsigned char *>(clone1->data())[0], 2)
        << "Clone1 lost data sharing";
}

// Test reference counting with explicit materialization
TEST_F(StorageTest, RefCountWithMaterialization)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone1 = cow::lazy_clone_storage(*original);
    auto clone2 = cow::lazy_clone_storage(*original);

    auto *ctx = static_cast<cow::COWDeleterContext *>(original->data_ptr().get_context());
    int initial_refcount = ctx->reference_count();

    // Materialize clone1
    cow::materialize_cow_storage(*clone1);

    EXPECT_EQ(ctx->reference_count(), initial_refcount - 1)
        << "Incorrect refcount after materialization";
}

// Test data access without materialization
TEST_F(StorageTest, DataAccess)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);

    // Fill with pattern
    for (size_t i = 0; i < 1000; i++)
    {
        static_cast<unsigned char *>(original->data())[i] = i % 256;
    }

    auto clone = cow::lazy_clone_storage(*original);

    // Reading without materialization
    EXPECT_EQ(std::memcmp(clone->data(), original->data(), 1000), 0)
        << "Data not identical in shared state";

    // Modify through clone without materializing
    static_cast<unsigned char *>(clone->data())[0] = 0xFF;

    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0], 0xFF)
        << "Modifications not visible without materialization";
}

// Test cloning materialized storage
TEST_F(StorageTest, CloneAfterMaterialize)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone1 = cow::lazy_clone_storage(*original);

    cow::materialize_cow_storage(*clone1);
    auto clone2 = cow::lazy_clone_storage(*clone1);

    std::memset(clone2->data(), 2, 1000);

    EXPECT_EQ(static_cast<unsigned char *>(clone1->data())[0], 2)
        << "Materialized storage not properly converted back to COW";
}

// Test multiple materializations
TEST_F(StorageTest, MultipleMaterialize)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone = cow::lazy_clone_storage(*original);

    cow::materialize_cow_storage(*clone);
    void *first_data = clone->data();

    cow::materialize_cow_storage(*clone);

    EXPECT_EQ(clone->data(), first_data)
        << "Multiple materializations caused unnecessary data copy";
}

// Test clone chain
TEST_F(StorageTest, CloneChain)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone1 = cow::lazy_clone_storage(*original);
    auto clone2 = cow::lazy_clone_storage(*clone1);
    auto clone3 = cow::lazy_clone_storage(*clone2);

    auto *ctx = static_cast<cow::COWDeleterContext *>(original->data_ptr().get_context());

    EXPECT_EQ(ctx->reference_count(), 4)
        << "Incorrect reference count in clone chain";

    cow::materialize_cow_storage(*clone2);
    std::memset(clone2->data(), 2, 1000);

    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0],
              static_cast<unsigned char *>(clone1->data())[0])
        << "Original and clone1 lost sharing after middle materialization";

    EXPECT_EQ(static_cast<unsigned char *>(original->data())[0],
              static_cast<unsigned char *>(clone3->data())[0])
        << "Original and clone3 lost sharing after middle materialization";
}

// Test cleanup
TEST_F(StorageTest, Cleanup)
{
    auto original = std::make_shared<Storage>(1000, cpu_device);
    auto clone1 = cow::lazy_clone_storage(*original);

    auto *ctx = static_cast<cow::COWDeleterContext *>(original->data_ptr().get_context());
    int initial_count = ctx->reference_count();

    {
        auto clone2 = cow::lazy_clone_storage(*original);
        EXPECT_EQ(ctx->reference_count(), initial_count + 1)
            << "Reference count not increased with nested clone";
    }

    EXPECT_EQ(ctx->reference_count(), initial_count)
        << "Reference count not decreased after clone destruction";
}