TARGETS=proxy client

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

proxy: proxy.cpp
	g++ -g -o proxy Response.cpp Request.cpp proxy.cpp -lpthread

client: client.cpp
	g++ -g -o $@ $<
