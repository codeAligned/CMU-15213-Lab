# Lab6 Proxy Lab

The original tar file is `proxylab-handout.tar` and the writeup is `proxylab.pdf`.

### Before starting 
Read lecture slides from `21` to `25` very carefully. They are very helpful when working on the lab.

### Part I: Sequential server
Tiny server and echo server, provided in the the associated code, are very useful for referring. Lecture notes also contain many code snippets that you can use.   
The only tricky part is parsing the URL. I used the code from a Stack Overflow answer. The detail is given in the documentation of `proxy.c`.  

### Part II: Concurrent server
Some small modification to the code in lecture slide and we get the full score.

### Part III: Cache proxy
I implemented a LRU cache based on hash. The synchronization and locking code from lecture slides can be directly used.

### Thoughts
This is a relatively easy lab. Understanding code in lecture slides and associated code examples help you greatly.

### Note
My solution is not heavily tested on web browser, which is actually suggested by the writeup :).



