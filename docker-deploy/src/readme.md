In this project, we make a http proxy which can do CONNECT, GET and POST and it can cache responses.

**Expiration Rules**

- If there is **max-age** cache control directive in the response, we would calculate the expiration time of this response by adding the max-age to **Date** header in the response, then compare it with current time. If current time pasts expiration tmie, it is expired. 
- If there is **Expires** header in the response, we would compare the current time with expire header time. If pasts, it is expired.
- If there no max-age directive nor Expires header, we would check if there is **Last-Modified** header field. If does, we would use heuristic freshness calculating mechanism to estimate the max age of the response ( ( last-modified time - date time )* 0.1 = approximate max-age) and then check if it is expired now. 

**Revalidation rules**

- If there is **no-cache** directive in cache-control header field, each tmie we need to use these this response we would revalidate it.
- If the response stored in cache is expired and it has **ETag** or **Last-Modified** header field, we would add the ETag or Last-Modified value in the request and send to server for re-validation.

**Cache policy**

- We use **LRU cache mechanism** and the capacity is **20**. When beyond the capacity, we would erase the least recent used one.
- If the response we recieve from the web server is **200 OK** and it is not **private** or **no-store**, we would save the response in the cache.
- If the response in cache is still fresh, we would send it back to server and leaves the response in cache. 
- If it is not fresh, we would ask the web server for revalidation. If proxy receives 304 Not Modified, we would still leave the response in cache and send it back to browser. If the response we get is 200 OK and it is cachable, we would replace the old response with new response and send new response back to sever.        

**Multiplexing**

- In our method connect, considering our proxy need to recv message from the web server and our browser, we use 'select' to make sure we can receieve all the mesage. And when we find out that the recv function returns 0, we believe the connect method in this thread has completed.

**Concurrency**

- We did spawn a thread to handle a request, and to do this, we set the global variable mutex, and whenever we receieve a leagal request, we will do pthread_create. And to avoid code collision, we use pthread_lock and pthread_unlock to make critical sections when we need to read/write from/to cache and write to log file.