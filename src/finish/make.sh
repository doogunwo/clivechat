g++ -o server server.cpp
g++ -o client client.cpp $(pkg-config gtkmm-3.0 --cflags  --libs)