CC = c++
CFLAGS = -std=c++1y -fPIC
DEPS = $(shell pkg-config --cflags --libs libmongocxx)

all: ./user_processor.so ./post_processor.so ./default_processor.so

./user_processor.so:
	$(CC) $(CFLAGS) ./src/user_processor.cpp ./src/DBManager.cpp -shared -o user_processor.so -lfastcgi-daemon2 $(DEPS)

./post_processor.so:
	$(CC) $(CFLAGS) ./src/post_processor.cpp ./src/DBManager.cpp -shared -o post_processor.so -lfastcgi-daemon2 $(DEPS)

./default_processor.so:
	$(CC) $(CFLAGS) ./src/default.cpp -shared -o default_processor.so -lfastcgi-daemon2 $(DEPS)

clean:
	rm -rf *.so

