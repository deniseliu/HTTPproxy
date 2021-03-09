**Danger log:**

- When we receive response from the web server, it loads too slow and and the page I receive seems not complete. Then i realized that the server may send several times per request, then I keep receiving response in a loop, and store the data I receive in a vector. We also check the length of the data we receive to judge whether the response is complete.
- Somtimes when we transfer large responses back to server, SIGPIPE occurs and our program exit. We may need adjust our send or store methodology in the future to fix this problem.

- In this project, our proxy need to be the client and the server at the same time. And in the processs of build the connnection between client and web server, our proxy need to create the socket, bind it, listen, and send and recv message. And errors can happen at eaxh process, so when we finish the bind, listen... we check if these functions work well, if not, we return immediately and print the responding error in our proxy log file.

- Since me and my teammate are all in China, and we can not visit websites like google, facebook, youtube... without VPN. So, in our testing process, we cannot send 200 OK back to google.com. But we sure our proxy works, since it works fine in other websites.

- When we first do multi-threading, we use std::thread to spawn a thread, but it turns out the connection is very slow, so we change to use pthread_create instead.

**Exception**:

- If there is some thing wrong when create socket or have error when recieve or send messgae, our code won't throw exceptions.

**Handle external failures**:

- 408 Request Timeout: if our proxy cannot connect the web server, it would return a 408 Request Timeout back to browser.
- 400 Bad Request: If our proxy receives a malformed request, for example, the format of request line is not correct, it would return 400 Bad Request back to browser.
- 502 Bad Gateway: I f our proxy receives a corrupted response from the web server, it would return 502 Bad Gateway back to the browser.
