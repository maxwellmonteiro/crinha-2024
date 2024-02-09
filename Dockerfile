FROM gcc:13.2
WORKDIR /usr/local/rinha-backend

COPY Makefile .
COPY src src
COPY include include
COPY lib lib

RUN make clean all