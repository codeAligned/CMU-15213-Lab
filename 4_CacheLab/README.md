# Lab4 CacheLab

The original tar file is `cachelab-handout.tar` and the writeup is `attacklab.pdf`. My solution is in `cachelab-handout`.
 
Read `6 Working on the Lab` in the writeup file before starting Part A, and Part B: these notes are at the end of the file and you might not notice them early enough. However, they provide useful information about the two tasks.  


### Part A 
PartA is relatively easy, if you remember the following: 
- When reading the address, use `long long` for possible long address, and use `%llx`, to read a `long long int` hex form. 
- As `s`, `E`, `b` are all constants determined at run time, you cannot construct an array directly. You have to use `malloc` to dynamically allocate memory on the heap. Note that, actually the compiler would not stop you from doing so, but the result is undefined. 

- You should be clear about the operations: `L`, `S`, and `M`.
    - `L`: load the data into cache. Possible outcome: `hit`, `miss`, `miss eviction`.  
    - `S`: store/write data from cache back to memory. If the data is already in the cache, write back. If the data is not originally in the cache, load the data into cache, and then write back. Possible outcome: `hit`, `miss`, `miss eviction`.
    - `M`: modify some data (load the data into cache, modify, and write back to memory), can be considered as a combination of `L` and `S`. Because after `L`, the data must be in the cache, so the outcome of `S` must be `hit`.  


### Part B
Part B is much harder. To solve this, you should recall how cache works: by extracting the set bits and tag bits from the address. Thus, the cache eviction behaves differently when dealing with Matrix of different size. For example, in 32x32 problem, the cache can contain at most 8 rows, so the 9th row would evict the 1st row; but in 64x64 problem, the cache can contain at most 4 rows, so the 5th row would evict the 1th row.  

Blocking technique is used for all problems in Part B.
  
Estimating/Computing cache miss is an important skill you pick up and get familar with when working on this part. 

There are a lot of notes and explanationn in the `trans.c` file.  

*Sidenote*: I found the 64x64 problem very hard and turned to Google for help. When searching in English, most results only contain code but no explanation at all. When searching in Chinese "cache lab 解答", a lot of useful blogs and answers come out.

- 32 x 32  
    This part is still handlable. The cache can contain at most 8 rows, so the block size cannot be larger than 8, otherwise cache misses occur even within the same block. It turns out that 8x8 blocking gives satisfying result.  

- 64 x 64  
    This part is much harder. The cache can contain at most 4 rows, so the block size cannot be larger than 4. Function `blocksize_4_64_64` applies the blocking naively but the result is 1699 misses, the reason is that since the blocking size is smaller than the size of the if cache line, cache is not fully utilizd. I improved the function to `blocksize_8_4_64_64`, where we consider four 4x4 blocks within a 8x8 block at the same time. The performance is better, with 1475 misses and gives me 6/8 points. I did not pursue further improvement, as I already spend a lot of time on it.

- 67 x 61  
    Much simpler than 64 x 64 case, as the number is not good. Simply apply the blocking naively and experiment with different blocking size. Block size of 8 and 16 both give satisfying result.  


### Thoughts
I spent around two hours on Part A, when I did not use `malloc` to allocate the cache data structure, but use `cache_line_t cache[S][E]` directly, even though the writeup explicity requires the code to use `malloc`. When using `cache_line_t cache[S][E]`, after I set the valid bits to some value, they become random value when I try to access them in other functions. Then I swithch to `malloc`, no such problem occurs, and I finish the problem in one or two hours.  
The lesson from this is that: First, when you need an array whose size is not known at compile time, use `malloc` to allocate memory dynamically. Second, and more importantly, do not be afraid to use `malloc` and pointers in C.   

I also wasted some time on Part B. Somehow at some point, I began to think that each row in the matrix would be mapped to a cache set and blablabla, which is obviously wrong. But, coincidently, I finished the 32 x 32 case successfylly, which is even worse because it took a very long time for me to find this great mistake in concept...   

The process is a bit painful, but really help me learn cache mechanism, blocking and caching performance compution. 
