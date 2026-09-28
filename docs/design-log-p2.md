# Design Log — Project 2

## Growth factor and amortized cost

For my `Conversation` class, I used a growth factor of 2. The conversation starts with a capacity of 0. When the first message is appended, the capacity becomes 1. After that, whenever `size_` reaches `capacity_`, I allocate a new array with twice the previous capacity. I copy the existing messages into the new array, delete the old array, and update `data_` and `capacity_`.

I chose doubling because it keeps `append()` amortized O(1). Most calls to `append()` only place a message at `data_[size_]` and increment `size_`, which is constant time. Reallocation is more expensive because all existing messages must be copied, but it does not happen on every append.

With doubling, the capacities grow as 1, 2, 4, 8, 16, and so on. For n appended messages, the total number of elements copied during reallocations is approximately

1 + 2 + 4 + 8 + ... < 2n.

Therefore, although an individual append that causes a reallocation can take O(n), the total reallocation work over n appends is O(n). Dividing that total work across the n append operations gives an amortized cost of O(1) per append. This avoids the O(n^2) behavior that would result from increasing the capacity by only one each time.

## Rule of Five evidence

`Conversation` owns a dynamically allocated `Message` array, so I implemented the Rule of Five to make ownership clear and prevent memory errors.

The destructor calls `delete[] data_`. Calling `delete[]` on `nullptr` is safe, which also makes destroying a moved-from conversation safe.

The copy constructor performs a deep copy. It copies `size_` and `capacity_`, allocates a separate `Message` array, and copies each stored message into that array. Because the copied conversation has a different `data_` pointer, the two objects do not share ownership.

The copy assignment operator also performs a deep copy. I first allocate and copy into a new array before deleting the object's current array. I also check `this != &other` to safely handle self-assignment.

For the move constructor, I copy the `data_`, `size_`, and `capacity_` values from the source without copying individual messages. I then set the source's pointer to `nullptr` and its size and capacity to zero. The move assignment operator follows the same ownership-transfer idea, while first deleting the destination's old array. This leaves the moved-from object valid and prevents both objects from deleting the same memory.

My tests check that copies have different backing-array addresses and that moves keep the original address while leaving the source empty.

## Sentinel scanner: bounded pending_ proof

The sentinel scanner must detect `<|end_conversation|>` even when it is split between arbitrary chunks. I use `pending_` to hold characters at the end of one chunk that might be the beginning of the sentinel.

Let the sentinel length be L. If no complete sentinel has been found, only the last L - 1 characters need to be held. Any earlier characters are safe to emit because a sentinel of length L cannot begin there and still require characters from a future chunk.

On each call to `feed()`, I combine `pending_` with the new chunk and search for the sentinel. If it is found, everything before it is safe text. If it is not found, I emit everything except the last L - 1 characters and store only those remaining characters in `pending_`.

Therefore, after every call to `feed()`,

`pending_.size() <= L - 1`.

The scanner does not need to store the entire model response. My tests also feed the sentinel one character at a time and test every possible two-chunk split to verify that chunk boundaries do not affect detection.

## What I would change differently

If I redesigned the scanner, I would consider using the Knuth-Morris-Pratt algorithm mentioned in the project specification. My current implementation is simpler and keeps the pending buffer bounded, but it combines strings and searches them again for each chunk. KMP could track the current matching position in the sentinel and avoid repeating comparisons when the input contains many partial matches. I kept the simpler approach for this project because it directly follows the required design and is easier to reason about and test.