FROM gcc:4.9
COPY . /Peer
WORKDIR /Peer/
RUN apt-get update
RUN apt-get install -y libsctp-dev
RUN gcc -pthread -o peer peer.c -lsctp