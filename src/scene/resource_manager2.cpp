#include "resource_manager2.hpp"

bool Handle::operator==(const Handle& other)
{
    return memcmp(this, &other, sizeof(Handle)) == 0;
}

void run_resoure_manager_fuzzer(size_t iterations, size_t pool_size)
{
    ResourceManager2<u32> manager;
    manager.init(pool_size);

    std::unordered_map<size_t, Handle> active_handles;
    std::vector<size_t> handle_keys;

    std::mt19937 rng(539);
    // 0-6: add, 7-9: remove, 10: validate
    std::uniform_int_distribution<int> action_dist(0, 10);

    for (size_t i = 0; i < iterations; ++i) {
        int action = action_dist(rng);

        // allocate
        if (action <= 6 || active_handles.empty()) {
            Handle handle = manager.create_handle();
            size_t unique_id = i;
            active_handles[unique_id] = handle;
            handle_keys.push_back(unique_id);
        }
        // deallocate
        else if (action <= 9) {
            size_t key_idx = std::uniform_int_distribution<size_t>(0, handle_keys.size() - 1)(rng);
            size_t unique_id = handle_keys[key_idx];

            manager.destroy_handle(active_handles[unique_id]);

            // validate
            if (manager.get(active_handles[unique_id]) != nullptr) {
                std::println("FUZZER ERROR: Handle at index {} should be invalid!", active_handles[unique_id].index);
                return;
            }

            active_handles.erase(unique_id);
            handle_keys.erase(handle_keys.begin() + key_idx);
        }
        // validate random
        else {
            if (!active_handles.empty()) {
                size_t key_idx = std::uniform_int_distribution<size_t>(0, handle_keys.size() - 1)(rng);
                Handle handle = active_handles[handle_keys[key_idx]];

                if (manager.get(handle) == nullptr) {
                    std::println("FUZZER ERROR: Active handle {} became null prematurely!", handle.index);
                    return;
                }
            }
        }

        if (i % 1000 == 0) {
            std::println("Iteration {}... OK", i);
        }
    }

    std::println("Fuzzing complete! No collisions or invalid states detected.");
}
