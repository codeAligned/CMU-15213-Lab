Please Read `6 Working on the Lab` in the writeup file before starting Part A, and Part B: these notes are at the end of the file and you might not notice them early enough. However, they provide useful information about the two tasks.

### Part A 
- As `s`, `E`, `b` are all constants determined at run time, you cannot construct an array directly. You have to use `malloc` to dynamically allocate memory on the heap. Note that, actually the compiler would not stop you from doing so, but the result is undefined. 

- You should be clear about the operations: `L`, `S`, and `M`.
    - `L`: load the data into cache. Possible outcome: `hit`, `miss`, `miss eviction`.  
    - `S`: store/write data from cache back to memory. If the data is already in the cache, write back. If the data is not originally in the cache, load the data into cache, and then write back. Possible outcome: `hit`, `miss`, `miss eviction`.
    - `M`: modify some data (load the data into cache, modify, and write back to memory), can be considered as a combination of `L` and `S`. Because after `L`, the data must be in the cache, so the outcome of `S` must be `hit`.